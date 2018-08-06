//
//  headset.h
//  CPlusPlusTest
//
//  Created by 黄剑 on 2018/8/5.
//  Copyright © 2018年 huangzhao. All rights reserved.
//

#ifndef headset_h
#define headset_h

#if defined(MACOS) || !defined(TIOCGWINSZ)
#include <sys/ioctl.h>

#endif

//c标准库
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//posix
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <glob.h>
#include <grp.h>
#include <netdb.h>
#include <pwd.h>
#include <regex.h>
#include <tar.h>
#include <termios.h>
#include <unistd.h>
#include <utime.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <poll.h>

#include <syslog.h>
//#include <ucontext.h>
#include <ulimit.h>

#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/uio.h>

#include <aio.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include <sys/msg.h>
#include <sys/statvfs.h>
#include <utmpx.h>

#if !__APPLE__
#include <mqueue.h>
#endif


#endif /* headset_h */
