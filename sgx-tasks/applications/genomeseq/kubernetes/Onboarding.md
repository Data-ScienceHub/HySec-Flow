https://www.youtube.com/watch?v=pNtqBGQ1b98&ab_channel=AbdullahBuhadod

My remote server spec https://ark.intel.com/content/www/us/en/ark/products/97469/intel-xeon-processor-e31240-v6-8m-cache-3-70-ghz.html

Jump start server
`ssh mi3se@usersrv02 -J mi3se@portal.cs.virginia.edu`

This onboarding was done on CentOS Linux 7, Intel(R) Xeon(R) CPU E3-1240 v6 @ 3.70GHz, Architecture x86_64 
Update the current OS environment
Check OS version first 
$ cat /etc/os-release 
Check this with the prerequisite list from https://github.com/intel/linux-sgx

```bash
sudo yum update
```

Install sgx driver from https://github.com/intel/linux-sgx-driver. Installs in "/opt/intel/sgxdriver"
```bash
# To check if matching kernel headers are installed
ls /usr/src/kernels/$(uname -r)

# https://unix.stackexchange.com/questions/215694/empty-kernel-directory-but-kernel-headers-are-installed
# https://askubuntu.com/questions/75709/how-do-i-install-kernel-header-files
# Update the kernel so that both kernel and kernel-devel are new.
# sudo yum upgrade

# Updating kernel is not always feasible. If that is the case install kernel-devel version that is same as the installed kernel version.
sudo yum install "kernel-devel-uname-r == $(uname -r)"

```

Faced error for 
```bash
sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"

# Sorry, user mi3se is not allowed to execute '/usr/bin/sh -c cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules' as root on usersrv02.
```

Verify installation is successful using bash command `ls /dev/isgx`

Facing [this issue](https://github.com/intel/linux-sgx/issues/441) when making [SampleCode](https://github.com/intel/linux-sgx/tree/master/SampleCode/Cxx11SGXDemo) 

## [Docker](https://docs.docker.com/engine/install/centos/)

Install the yum-utils package (which provides the yum-config-manager utility) and set up the stable repository.

```bash
sudo yum install -y yum-utils
sudo yum-config-manager \
    --add-repo \
    https://download.docker.com/linux/centos/docker-ce.repo
```

Install the latest version of Docker Engine and containerd, or go to the next step to install a specific version:

```bash
sudo yum install docker-ce docker-ce-cli containerd.io
```
If prompted to accept the GPG key, verify that the fingerprint matches 060A 61C5 1B55 8A7F 742B 77AA C52F EB6B 621E 9F35, and if so, accept it. This command installs Docker, but it doesn’t start Docker. It also creates a docker group, however, it doesn’t add any users to the group by default.

Start Docker. Then verify that Docker Engine is installed correctly by running the hello-world image.

```bash
sudo systemctl start docker
sudo docker run hello-world
```

## [Kubernetes](https://kubernetes.io/docs/setup/)
Install kubectl binary with curl on Linux. Install kubectl. Test to ensure the version you installed is up-to-date.
```bash
curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"

sudo install -o root -g root -m 0755 kubectl /usr/local/bin/kubectl

kubectl version --client
```

## [Minikube](https://minikube.sigs.k8s.io/docs/start/)
minikube is local Kubernetes, focusing on making it easy to learn and develop for Kubernetes.

All you need is Docker (or similarly compatible) container or a Virtual Machine environment, and Kubernetes is a single command away: `minikube start`.

What you’ll need

* 2 CPUs or more
* 2GB of free memory
* 20GB of free disk space
* Internet connection
* Container or virtual machine manager, such as: Docker, Hyperkit, Hyper-V, KVM, Parallels, Podman, VirtualBox, or VMWare

To install the latest minikube stable release on x86-64 Linux using binary download:

```bash
curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
sudo install minikube-linux-amd64 /usr/local/bin/minikube
```


https://gist.github.com/mjuric/c519d470eac60b08de5ed735ff5a2ef9

## Additional Configuration
First try running docker without sudo. This is needed for minikube to use it as driver. If it runs successfully, you'll see `Hello from Docker!` log on console.
```bash
docker run hello-world

```

If it doesn't run without sudo and gives the following error, try this [link](https://www.digitalocean.com/community/questions/how-to-fix-docker-got-permission-denied-while-trying-to-connect-to-the-docker-daemon-socket)
> Exiting due to PROVIDER_DOCKER_NEWGRP: "docker version --format -" exit status 1: Got permission denied while trying to connect to the Docker daemon socket at unix:///var/run/docker.sock: Get "http://%2Fvar%2Frun%2Fdocker.sock/v1.24/version": dial unix /var/run/docker.sock: connect: permission denied
💡  Suggestion: Add your user to the 'docker' group: 'sudo usermod -aG docker $USER && newgrp docker'

First create a docker group and then add user to that. You would need to log out and log back (or restart machine) in so that your group membership is re-evaluated. Also restart the docker if needed.
```bash
sudo groupadd docker
sudo usermod -aG docker ${USER}
sudo systemctl restart docker

```

If it still doesn't work try `sudo chmod 666 /var/run/docker.sock`. Now you should be able to run docker without sudo.


```bash
systemctl start docker
minikube start --driver=docker
```

# Running Linux-sgx
```
git clone https://github.com/intel/linux-sgx.git --recurse-submodules

sudo yum groupinstall 'Development Tools'
sudo yum install ocaml ocaml-ocamlbuild wget python2 openssl-devel git cmake perl

sudo alternatives --set python /usr/bin/python2
# gets error: cannot access /var/lib/alternatives/python: No such file or directory
# fix, check python version. python would be python2 and python3 points to the later version.
python --version

# Use the following command to install additional required tools and latest Intel(R) SGX SDK Installer to build the Intel(R) SGX PSW
sudo yum install openssl-devel libcurl-devel protobuf-devel cmake rpm-build createrepo yum-utils

# Download the source code and prepare the submodules and prebuilt binaries
git clone https://github.com/intel/linux-sgx.git
cd linux-sgx && make preparation

# Copy the mitigation tools corresponding to current OS distribution from external/toolset/{current_distr} to /usr/local/bin and make sure they have execute permission

sudo cp external/toolset/{current_distr}/{as,ld,ld.gold,objdump} /usr/local/bin
# error: ld.gold doesn't exist
# fix: run the following instead
sudo cp external/toolset/centos8.2/{ar,as,ld,objcopy,objdump,ranlib} /usr/local/bin
which as ld ld.gold objdump
```

Install the Intel(R) SGX SDK
To install the Intel(R) SGX SDK, invoke the installer, as follows:

$ cd linux/installer/bin

Error: I wasn't able to make the sdk for some compile error. So I downloaded them from https://download.01.org/intel-sgx/sgx-linux/2.15/distro/centos8.2-server/ using wget command. Then make then executable using `chmod a+x filename`

$ ./sgx_linux_x64_sdk_${version}.bin
NOTE: You need to set up the needed environment variables before compiling your code. To do so, run:

$ source ${sgx-sdk-install-path}/environment

Error: I found there's no environment file in that path. 
Fix: sgx sdk has to be installed using sudo.  

$ sudo ./sgx_linux_x64_sdk_${version}.bin

Write no in Do you want to install in current directory? [yes/no]. Then provide /opt/intel/ as the path. After successfully installing set the environment source
$ source /opt/intel/sgxsdk/environment 

Install the sgx driver from. First install the requirements from https://github.com/intel/linux-sgx-driver. To install matching headers 
$ sudo apt-get install linux-headers-$(uname -r)

Error: E: Unable to locate package linux-headers-3.....

By default it'll be installed in  "/opt/intel/sgxdriver"
$ sudo ./sgx_linux_x64_driver_2.11.0_2d2b795.bin

changing directory into sgxsdk/SampleCode/LocalAttestion and trying to compile in simulation mode
$ cd ${sgx-sdk-install-path}/SampleCode/LocalAttestation
$ make SGX_MODE=SIM

Error: version `GLIBC_2.28` not found. 
Tried `sudo yum update`. `ldd version` returned my glibc version to be 2.17. But sgx requires minimum version 2.27. 

$ cd bin
$ ./app

```
# Goto linux-sgx/SampleCode/SampleEnclave/
# try running the following for running demo codes into simulation mode
make SGX_MODE=SIM

# this might end with the following error https://github.com/intel/linux-sgx/issues/652
/usr/bin/ld: warning: libsgx_uae_service_sim.so, needed by /opt/intel/sgxsdk/lib64/libsgx_urts_sim.so, not found (try using -rpath or -rpath-link)
/opt/intel/sgxsdk/lib64/libsgx_urts_sim.so: undefined reference to `get_launch_token'

# It got resolved after setting up the environment variable before compiling.
source /opt/intel/sgxsdk/environment
```

Running `make` for a sample code returns this error
```
Fatal error: exception Sys_error("./Enclave_u.h: Permission denied")
make[1]: *** [Makefile:219: App/Enclave_u.h] Error 2
make[1]: Leaving directory '/linux-sgx/Git/linux-sgx/SampleCode/Cxx11SGXDemo'
make: *** [Makefile:181: all] Error 2
```

## Install the Intel(R) SGX PSW
```bash
# didn't work
sudo dnf --enablerepo=PowerTools install libcurl-devel protobuf-devel
sudo dnf config-manager --set-enabled PowerTools install libcurl-devel protobuf-devel

```

# Setting up storage
[Persistent Volumes on Kubernetes for beginners](https://www.youtube.com/watch?v=ZxC6FwEc9WQ&ab_channel=ThatDevOpsGuy)

## NFS
* [How to Setup NFS Server On Red Hat Enterprise Linux 8 / CentOS 8](https://www.youtube.com/watch?v=W1R4p6326uQ&ab_channel=LyncSoftServices)
* 

## Glusterfs
Youtube playlist : [GlusterFS Storage for Beginners](https://www.youtube.com/playlist?list=PL34sAs7_26wOwCvth-EjdgGsHpTwbulWq)


Done following https://www.youtube.com/watch?v=8CJbyBdxcYU
```bash
sudo dnf install -y centos-release-gluster

# search for glusterfs-server
dnf search glusterfs

# look for glusterfs-server.x86_64 in the search results. then install that one
# --best --allowerasing will install the best combinations of libraries erasing conflicting ones
sudo dnf install -y glusterfs-server --best --allowerasing

# systemctl start glusterd

```


# Running HysecFlow

If you are running on the local pc, in a cmd line run
$ minikube start

Mount a local directory to minikube. For example, to mount D:\shared directory to /shared folder in minikube from cmd run
https://minikube.sigs.k8s.io/docs/handbook/mount/
$ minikube mount D://shared:/shared

Now you can add this mounted path as hostPath in yaml file. Related issues https://github.com/kubernetes/kubernetes/issues/59876 and https://stackoverflow.com/questions/65245357/mapping-local-directory-to-kubernetes. Keep in mind the persistant volumes to write https://minikube.sigs.k8s.io/docs/handbook/persistent_volumes. 

This should open the GUI in the default browser. If you are running minikube in the remote server, follow https://stackoverflow.com/questions/47173463/how-to-access-local-kubernetes-minikube-dashboard-remotely. Create a new cmd line and run
$ minikube dashboard

Then in another cmd line in the remote server run
$ kubectl proxy

In my local pc run
$ ssh -L 12345:localhost:8001 myLogin@myRemoteServer

The dashboard was then available at this url on my pc:

http://localhost:12345/api/v1/namespaces/kubernetes-dashboard/services/http:kubernetes-dashboard:/proxy/

In terminal opened in vs code editor execute
$ kubectl apply -f .\applications\genomeseq\kubernetes\server.yaml

This will probably fail because of missing args. It is in the file, but for some bug doesn't get loaded. Open minikube dashboard GUI in the web brower.

After getting the GUI, goto deployments and edit server. Then add the missing args (args: ["cd /sgx-tasks/ && git pull && cd /sgx-tasks/build && make && /sgx-tasks/build/applications/genomeseq/server"]) after the command (approx line 160-163), then restart deployment. 

Only after server succeeds deploy worker nodes.

```bash
# check deployed pods
kubectl get pods

# get the list of pods and then log into the server pod using server pod name
kubectl exec --stdin --tty tasker-server-6fd69d8888-4l5lm -- /bin/bash

kubectl exec --stdin --tty tasker-server-587f57bd59-5sd8x -- /bin/bash

# Now you are into the server pod. cd to client code folder
cd /sgx-tasks/build/applications/genomeseq/

# run client using a unique id. to connect from outside you also need to give driver port. But as you are already inside 
# driver port, its unnecessary
./client client

# list data and partitions
ls
# now that you have seen it works, just exit using CTRL+C. 
# Then check files, you should see a 3.1G size file hg38.fa
# or download it from https://hgdownload.cse.ucsc.edu/goldenpath/hg38/bigZips/. then unzip it using `gzip -d hg38.fa.gz`.
# another library like BWA https://hub.docker.com/r/gelog/snap
# download ref genome from https://www.internationalgenome.org/data-portal/search?q=SRR062634
ls -lah /data/

# Now log into client again . Then partition and index the genome file. Here we are creating 4 partitions.
./client client1
index -p 1 -s hg38.fa

# Check log of client pod to see how it partitioned the data. 
# If we scale up the worker node to have multiple node, 
```
