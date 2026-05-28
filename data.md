
服务器资源信息：


banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ lscpu
Architecture:             x86_64
  CPU op-mode(s):         32-bit, 64-bit
  Address sizes:          46 bits physical, 48 bits virtual
  Byte Order:             Little Endian
CPU(s):                   2
  On-line CPU(s) list:    0,1
Vendor ID:                GenuineIntel
  Model name:             Intel(R) Xeon(R) Platinum
    CPU family:           6
    Model:                85
    Thread(s) per core:   2
    Core(s) per socket:   1
    Socket(s):            1
    Stepping:             4
    BogoMIPS:             4999.99
    Flags:                fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc rep_good
                           nopl xtopology nonstop_tsc cpuid tsc_known_freq pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c
                           rdrand hypervisor lahf_lm abm 3dnowprefetch cpuid_fault invpcid_single pti fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid avx512f avx512dq rdseed a
                          dx smap clflushopt clwb avx512cd avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves arat
Virtualization features:  
  Hypervisor vendor:      KVM
  Virtualization type:    full
Caches (sum of all):      
  L1d:                    32 KiB (1 instance)
  L1i:                    32 KiB (1 instance)
  L2:                     1 MiB (1 instance)
  L3:                     33 MiB (1 instance)
NUMA:                     
  NUMA node(s):           1
  NUMA node0 CPU(s):      0,1
Vulnerabilities:          
  Gather data sampling:   Unknown: Dependent on hypervisor status
  Itlb multihit:          KVM: Mitigation: VMX unsupported
  L1tf:                   Mitigation; PTE Inversion
  Mds:                    Vulnerable: Clear CPU buffers attempted, no microcode; SMT Host state unknown
  Meltdown:               Mitigation; PTI
  Mmio stale data:        Vulnerable: Clear CPU buffers attempted, no microcode; SMT Host state unknown
  Reg file data sampling: Not affected
  Retbleed:               Vulnerable
  Spec rstack overflow:   Not affected
  Spec store bypass:      Vulnerable
  Spectre v1:             Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:             Mitigation; Retpolines; STIBP disabled; RSB filling; PBRSB-eIBRS Not affected; BHI Retpoline
  Srbds:                  Not affected
  Tsx async abort:        Not affected

banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ free -h
               total        used        free      shared  buff/cache   available
Mem:           1.6Gi       262Mi       400Mi       2.0Mi       979Mi       1.2Gi
Swap:             0B          0B          0B
banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ 


banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ lsblk 
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
loop0    7:0    0 61.9M  1 loop /snap/core20/1405
loop1    7:1    0 79.9M  1 loop /snap/lxd/22923
loop2    7:2    0 63.8M  1 loop /snap/core20/2866
loop3    7:3    0 49.3M  1 loop /snap/snapd/26865
loop4    7:4    0 91.7M  1 loop /snap/lxd/38800
vda    252:0    0   40G  0 disk 
├─vda1 252:1    0    1M  0 part 
├─vda2 252:2    0  200M  0 part /boot/efi
└─vda3 252:3    0 39.8G  0 part /
banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ 




banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 00:16:3e:01:d8:6b brd ff:ff:ff:ff:ff:ff
    altname enp0s5
    altname ens5
    inet 172.18.48.158/18 metric 100 brd 172.18.63.255 scope global dynamic eth0
       valid_lft 1892084813sec preferred_lft 1892084813sec
    inet6 fe80::216:3eff:fe01:d86b/64 scope link 
       valid_lft forever preferred_lft forever
banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ 

banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ uname -a 
Linux iZmj7jdfa1rfhovd3ord7vZ 5.15.0-142-generic #152-Ubuntu SMP Mon May 19 10:54:31 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux
banju@iZmj7jdfa1rfhovd3ord7vZ:~/Mini-httpserver/build$ 



测试结果：
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c1000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    16.39ms    4.41ms  53.06ms   70.26%
    Req/Sec    15.31k   668.95    16.93k    77.50%
  Latency Distribution
     50%   16.24ms
     75%   18.42ms
     90%   21.84ms
     99%   29.69ms
  1828243 requests in 30.06s, 214.46MB read
Requests/sec:  60827.23
Transfer/sec:      7.14MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ 

banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ wrk -t4 -c5000 -d30s --latency http://localhost:8080/hello 
Command 'wrk' not found, but can be installed with:
sudo apt install wrk
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c5000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    82.46ms   16.68ms 145.64ms   67.56%
    Req/Sec    15.19k     1.89k   26.20k    70.71%
  Latency Distribution
     50%   82.32ms
     75%   94.72ms
     90%  102.47ms
     99%  122.49ms
  1807743 requests in 30.09s, 212.05MB read
Requests/sec:  60086.93
Transfer/sec:      7.05MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c10000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   174.22ms   36.58ms 360.32ms   69.13%
    Req/Sec    14.34k     3.91k   25.25k    70.37%
  Latency Distribution
     50%  165.59ms
     75%  200.26ms
     90%  224.04ms
     99%  263.04ms
  1699416 requests in 30.09s, 199.34MB read
Requests/sec:  56472.42
Transfer/sec:      6.62MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c15000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 15000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   294.94ms   64.32ms 529.42ms   70.89%
    Req/Sec    12.70k     3.66k   23.37k    65.62%
  Latency Distribution
     50%  293.39ms
     75%  339.39ms
     90%  374.66ms
     99%  460.67ms
  1501254 requests in 30.10s, 176.10MB read
Requests/sec:  49879.05
Transfer/sec:      5.85MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c20000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 20000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   427.16ms   76.59ms 643.83ms   70.85%
    Req/Sec    11.67k     4.07k   25.66k    68.31%
  Latency Distribution
     50%  419.34ms
     75%  482.84ms
     90%  526.73ms
     99%  604.47ms
  1372004 requests in 30.11s, 160.94MB read
Requests/sec:  45566.35
Transfer/sec:      5.35MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c25000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 25000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   547.78ms  110.88ms 858.56ms   68.92%
    Req/Sec    11.38k     4.22k   26.71k    69.73%
  Latency Distribution
     50%  552.49ms
     75%  623.71ms
     90%  688.91ms
     99%  805.41ms
  1318662 requests in 30.05s, 154.68MB read
Requests/sec:  43881.28
Transfer/sec:      5.15MB
banju@iZmj7jdfa1rfhovd3ord7vZ:~/download/wrk$ ./wrk -t4 -c30000 -d30s --latency http://localhost:8080/hello 
Running 30s test @ http://localhost:8080/hello
  4 threads and 30000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   695.31ms  113.10ms 965.88ms   69.74%
    Req/Sec    11.09k     4.97k   28.00k    69.57%
  Latency Distribution
     50%  702.16ms
     75%  775.44ms
     90%  832.06ms
     99%  913.59ms
  1250729 requests in 30.09s, 146.71MB read
Requests/sec:  41561.16
Transfer/sec:      4.88MB
