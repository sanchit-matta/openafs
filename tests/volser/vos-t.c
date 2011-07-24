#include <afsconfig.h>
#include <afs/param.h>

#include <roken.h>

#include <rx/rx.h>
#include <ubik.h>

#include <afs/com_err.h>
#include <afs/vldbint.h>
#include <afs/cellconfig.h>

#include <tap/basic.h>

#include "common.h"

/* This checks for a bug in vos where it would fail to allocate additional
 * space for the results of multi homed VL_GetAddrsU, and so would segfault
 * if a host with a small number of addresses was immediately followed by
 * a host with a large number of addresses.
 */
void
TestListAddrs(struct ubik_client *client, char *dirname)
{
    int code;
    bulkaddrs addrs;
    pid_t pid;
    int outpipe[2];
    int status;
    char *buffer;
    size_t len;

    afs_uint32 addrsA[] = {0x0a000000};
    afs_uint32 addrsB[] = {0x0a000001, 0x0a000002, 0x0a000003, 0x0a000004,
			   0x0a000005, 0x0a000006, 0x0a000007, 0x0a000008,
			   0x0a000009, 0x0a00000a, 0x0a00000b, 0x0a00000c,
			   0x0a00000d, 0x0a00000e, 0x0a00000f};
    char uuidA[] = {0x4F, 0x44, 0x94, 0x47, 0x76, 0xBA, 0x47, 0x2C, 0x97, 0x1A,
		    0x86, 0x6B, 0xC0, 0x10, 0x1A, 0x4B};
    char uuidB[] = {0x5D, 0x2A, 0x39, 0x36, 0x94, 0xB2, 0x48, 0x90, 0xA8, 0xD2,
		    0x7F, 0xBC, 0x1B, 0x29, 0xDA, 0x9B};
    char expecting[] = "10.0.0.0\n10.0.0.1\n10.0.0.2\n10.0.0.3\n10.0.0.4\n"
		       "10.0.0.5\n10.0.0.6\n10.0.0.7\n10.0.0.8\n10.0.0.9\n"
		       "10.0.0.10\n10.0.0.11\n10.0.0.12\n10.0.0.13\n"
		       "10.0.0.14\n10.0.0.15\n";

    addrs.bulkaddrs_len = 1;
    addrs.bulkaddrs_val = addrsA;
    code = ubik_VL_RegisterAddrs(client, 0, (afsUUID *)uuidA, 0, &addrs);
    is_int(0, code, "First address registration succeeds");

    addrs.bulkaddrs_len = 15;
    addrs.bulkaddrs_val = addrsB;
    code = ubik_VL_RegisterAddrs(client, 0, (afsUUID *)uuidB, 0, &addrs);
    is_int(0, code, "Second address registration succeeds");

    /* Now we need to run vos ListAddrs and see what happens ... */
    pipe(outpipe);
    pid = fork();
    if (pid == 0) {
	dup2(outpipe[1], STDOUT_FILENO); /* Redirect stdout into pipe */
	close(outpipe[0]);
	close(outpipe[1]);

	execl("../../src/volser/vos", "vos",
	      "listaddrs", "-config", dirname, "-noauth", NULL);
    }
    close(outpipe[1]);
    buffer = malloc(4096);
    len = read(outpipe[0], buffer, 4096);
    waitpid(pid, &status, 0);
    buffer[len]='\0';
    is_string(expecting, buffer, "vos output matches");
    free(buffer);
}

int
main(int argc, char **argv)
{
    char *dirname;
    struct afsconf_dir *dir;
    struct afsconf_cell info;
    int code;
    int i;
    pid_t serverPid;
    struct rx_securityClass *secClass;
    struct rx_connection *serverconns[MAXSERVERS];
    struct ubik_client *ubikClient = NULL;
    int secIndex;

    plan(5);

    code = rx_Init(0);

    dirname = afstest_BuildTestConfig();

    dir = afsconf_Open(dirname);

    code = afstest_AddDESKeyFile(dir);
    if (code) {
	afs_com_err("vos-t", code, "while adding test DES keyfile");
	exit(1);
    }

    code = afstest_StartVLServer(dirname, &serverPid);
    if (code) {
	afs_com_err("vos-t", code, "while starting the vlserver");
	exit(1);
    }

    /* Let it figure itself out ... */
    sleep(5);

    code = afsconf_ClientAuthSecure(dir, &secClass, &secIndex);
    if (code) {
	afs_com_err("vos-t", code, "while getting a fake token");
	exit(1);
    }

    code = afsconf_GetCellInfo(dir, NULL, AFSCONF_VLDBSERVICE, &info);
    if (code) {
	afs_com_err("vos-t", code, " while getting addresses from cellservdb");
	exit(1);
    }
    ok(info.numServers < MAXSERVERS, "Number of servers is within range");

    for (i = 0; i < info.numServers; i++)
	serverconns[i] = rx_NewConnection(info.hostAddr[i].sin_addr.s_addr,
					  info.hostAddr[i].sin_port,
					  USER_SERVICE_ID,
					  secClass, secIndex);

    code = ubik_ClientInit(serverconns, &ubikClient);

    TestListAddrs(ubikClient, dirname);

    code = afstest_StopVLServer(serverPid);
    is_int(0, code, "Server exited cleanly");

    return 0;
}