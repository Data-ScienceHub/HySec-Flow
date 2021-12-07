https://community.intel.com/t5/Intel-Software-Guard-Extensions/Error-Bad-return-status-for-module-build-on-kernel-4-18-0-305-19/td-p/1324958

https://download.01.org/intel-sgx/latest/dcap-latest/linux/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf

https://github.com/intel/SGXDataCenterAttestationPrimitives/blob/master/README.md

# Quick start

[Intel SGX tutorials : Setup & Run Simple SGX appliaction in Simulation mode](https://www.youtube.com/watch?v=pNtqBGQ1b98&ab_channel=AbdullahBuhadod)

[Intel SGX Tutorials : (2) Creating helloworld ECALL function inside the enclave](https://youtu.be/6l-ULyhBbfg)

# My remote servers

My remote server spec https://ark.intel.com/content/www/us/en/ark/products/97469/intel-xeon-processor-e31240-v6-8m-cache-3-70-ghz.html

Jump start server
`ssh mi3se@usersrv02 -J mi3se@portal.cs.virginia.edu`

`ssh mdkislam@bio-sgx02 -J mdkislam@sharks.luddy.indiana.edu`

This onboarding was done on CentOS Linux 7, Intel(R) Xeon(R) CPU E3-1240 v6 @ 3.70GHz, Architecture x86_64 
Update the current OS environment
Check OS version first 
$ cat /etc/os-release 
Check this with the prerequisite list from https://github.com/intel/linux-sgx

For bio-sgx nodes, if you are not connected to the indiana university network either first connect to VPN or connect to a publicly available cluster. Then connect to the bio-sgx nodes from there. For me, I connected to the public cluster sharks.luddy.indiana.edu.

```
ssh mdkislam@sharks.luddy.indiana.edu
ssh bio-sgx02.luddy.indiana.edu
```

Intially make sure the OS is updated.

```bash
sudo yum update
```

# [Intel SGX driver](https://github.com/intel/linux-sgx-driver)

Starting with Linux kernel 5.11 (you can find kernel version using `uname -r`), you do not need to install an SGX drivers anymore. Hence, we recommend to update your kernel instead of using an SGX driver. But if you have an older kernel and can't upgrade, check whether your processor supports Flexible Launch Control (FLC) feature following [this](https://www.intel.com/content/www/us/en/support/articles/000057420/software/intel-security-products.html) or just use command `cpuid | grep -i sgx`.

* Install DCAP driver (sgx_linux_x64-driver_1.41+.x.bin) if is has FLC support.
* Otherwise install the regular, out-of-tree (OOT) driver (sgx_linux_x64_driver_2.11+.x.bin).
* Attempting to install the out-of-tree (OOT) driver (sgx_linux_x64_driver_2.11+.x.bin), via the binary or building from source, will [cause a conflict](https://community.intel.com/t5/Intel-Software-Guard-Extensions/Intel-SGX-driver-issue-with-i5-7300U-CPU/td-p/1273229) with the SGX in-kernel driver.
* Try the bash script given in [Sconedocs](https://sconedocs.github.io/sgxinstall/) for easier installation.
  * Download the script as follows:

    ```bash
    curl -fssl https://raw.githubusercontent.com/scontain/SH/master/install_sgx_driver.sh --output install_sgx_driver.sh
    chmod u+x install_sgx_driver.sh
    ```

  * The checkcommand tests if an up-to-date SGX driver and some required extension are installed. Typically, you would execute as follows: 
  
    ```bash
    ./install_sgx_driver.sh check
    ```

  * Supports both type of installations

    ```bash
      # install SGX driver with metrics extension; replaces any existing SGX driver
    ./install_sgx_driver.sh install --force -p metrics

    # install DCAP SGX driver with metrics extension; replaces any existing SGX driver
    ./install_sgx_driver.sh install --dcap --force -p metrics -p version 
    ```

* Follow the [installation guideline](https://download.01.org/intel-sgx/latest/dcap-latest/linux/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf).
* The software installs in the "/opt/intel/sgxdriver" folder.

I found my kernel driver was `4.18.0-348.2.1.el8_5.x86_64`.

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

```bash
sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"

# Sorry, user mi3se is not allowed to execute '/usr/bin/sh -c cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules' as root on usersrv02.
```

Verify installation is successful using bash command `ls /dev/isgx`

Facing [this issue](https://github.com/intel/linux-sgx/issues/441) when making [SampleCode](https://github.com/intel/linux-sgx/tree/master/SampleCode/Cxx11SGXDemo) 

* [SGX driver fails to install on Ubuntu 18.04](https://github.com/microsoft/oe-engine/issues/61)
* [Can't run hardware sample code](https://github.com/intel/linux-sgx/issues/686)

## 1. Check if the Intel SGX driver is installed

```bash
sudo /sbin/depmod

# important: this needs to run after each reboot
sudo /sbin/modprobe isgx
```

If you get an error, the Intel SGX driver was not correctly installed.

## .2 Install the Intel SGX driver

Download the desired SGX driver from the latest Intel SGX Linux Driver Repository (As an example, driver_2.6.0_b0a445.bin is the regular SGX driver; driver_1.36.bin is the DCAP driver).
Execute the bin file: sudo ./sgx_linux_x64_driver_2.6.0_b0a445b.bin.
You should see a Installation is successful! message.
Confirm that the driver was installed: ls /dev | grep sgx.

## 3. Start the aesmd service

The Intel(R) SGX PSW installer installs an aesmd service in your machine, which is running in a special linux account aesmd.

```bash
ps aux | grep -i aesm
sudo systemctl enable aesmd
sudo systemctl start aesmd

sudo cat /var/log/syslog | grep -i aesm
# this might show aesm is having trouble connecting to network using white list
```

Found the following error with `sudo systemctl status aesmd`

```bash
]: [ADMIN]White List update requested
]: [ADMIN]White List update failed due to network error
]: [load_qe ../qe_logic.cpp:642] Error, call sgx_create_enclave QE fail [load_qe], SGXError:4004.
```

### Configure the Proxy for aesmd Service

The aesmd service uses the HTTP protocol to initialize some services.
If a proxy is required for the HTTP protocol, you may need to manually set up the proxy for the aesmd service.
You should manually edit the file /etc/aesmd.conf (refer to the comments in the file) to set the proxy for the aesmd service.
After you configure the proxy, you need to restart the service to enable the proxy.

Note: [related issue](https://github.com/intel/sgx-ra-sample/issues/16). No direction given here about proxy server address. 

Using `sudo systemctl status aesmd` I found `[ADMIN]White List update failed due to network error`. For now I have set the proxy to system default by editing the aesmd.conf file and restarting the service. This resolves the network error.

But there is still the following issue

```bash
[load_qe ../qe_logic.cpp:642] Error, call sgx_create_enclave QE fail [load_qe], SGXError:4004.
Failed to load QE3: 0x4004
```

### Launching an Enclave with Provision Bit Set

#### Background

An enclave may set the provision bit in its attributes to be able to request provision key. Acquiring provision key may have privacy implications and should be limited. Such enclaves are referred to as provisioning enclaves below.

For applications loading provisioning enclaves, the platform owner (administrator) must grant provisioning access to the app process as described below.

Note for Intel Signed Provisioning Enclaves: The Intel(R) SGX driver before V1.41 allows Intel(R)’s provisioning enclaves to be launched without any additional permissions. But the special treatment for Intel signed enclaves is removed in the driver starting from V1.41 release to be aligned with the upstream kernel changes. If you upgrade driver from versions older than V1.41 or switch to the mainline kernel (5.11 or above), please make sure apps loading Intel signed provisioning enclaves have the right permissions as described below.

#### Driver Settings

The Intel(R) SGX driver installation process described above creates 2 new devices on the platform, and setup these devices with the following permissions:

crw-rw-rw- root root      /dev/sgx/enclave
crw-rw---- root sgx_prv   /dev/sgx/provision
Note: The driver installer BIN file provided by Intel(R) automatically copy the udev rules and run udevadm trigger to activate the rules so that the permissions are set as above.

This configuration enables every user to launch an enclave, but only members of the sgx_prv group are allowed to launch an enclave with provision bit set. Failing to set these permissions may prevent processes that are not running under root privilege from launching a provisioning enclave.

#### Process Permissions and Flow

As mentioned above, Intel(R) provisioning enclaves are signed with Intel(R) owned keys that will enable them to get launched unconditionally. A process that launches other provisioning enclaves is required to use the SET_ATTRIBUTE IOCTL before the INIT_ENCLAVE IOCTL to notify the driver that the enclave being launched requires provision key access. The SET_ATTRIBUTE IOCTL input is a file handle to /dev/sgx/provision, which will fail to open if the process doesn't have the required permission. To summarize, the following flow is required by the platform admin and a process that require provision key access:

#### Software installation flow

Add the user running the process to the sgx_prv group:
`sudo usermod -a -G sgx_prv user name`
Enclave launch flow:

* Create the enclave
* Open handle to /dev/sgx/provision
* Call SET_ATTRIBUTE with the handle as a parameter
* Add pages and initialize the enclave
  
Note: The Enclave Common Loader library is following the above flow and launching enclave based on it, failure to grant correct access to the launching process will cause a failure in the enclave initialization.

### [Create and add user to sgx_prv group](https://github.com/intel/SGXDataCenterAttestationPrimitives/tree/master/driver/linux)

```bash
sudo cp  10-sgx.rules /etc/udev/rules.d
sudo groupadd sgx_prv
sudo udevadm trigger

# add users to sgx_prv group
sudo usermod -a -G sgx_prv $USER
sudo usermod -a -G sgx_prv aesmd
```

## [Docker](https://docs.docker.com/engine/install/rhel/)

Before installing docker clean the old versions

```bash
sudo yum remove docker \
                  docker-client \
                  docker-client-latest \
                  docker-common \
                  docker-latest \
                  docker-latest-logrotate \
                  docker-logrotate \
                  docker-engine

# if you face conflicts with podman packages or containers-common
# https://techexperience.me/2020/04/16/docker-conflicts-with-file-from-package-podman-manpages-on-centos-linux-release-8-1-1911-core/
sudo yum remove -y podman-manpages containers-common

```

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

This command installs Docker, but it doesn’t start Docker. It also creates a docker group, however, it doesn’t add any users to the group by default.

Start Docker. Then verify that Docker Engine is installed correctly by running the hello-world image.

```bash
sudo systemctl start docker
sudo docker run hello-world
# enable docker service
systemctl enable docker.service
```

Build docker image from a file. The dot at last mean to build at current directory. https://docs.docker.com/engine/reference/commandline/build/
$ docker build -f filepath .
Build image with a name tag
$ docker build -t name -f filepath .

## Cleaning
https://www.digitalocean.com/community/tutorials/how-to-remove-docker-images-containers-and-volumes
clean up any resources — images, containers, volumes, and networks — that are dangling (not tagged or associated with a container)
$ docker system prune
To additionally remove any stopped containers and all unused images (not just dangling images), add the -a flag to the command:
$ docker system prune -a

To remove image, first find the image id, then use that to remove the image
$ docker images
$ docker rmi image-id

To remove a container, first find container id, then use that
$ docker container list

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
$ wget --recursive --no-parent https://download.01.org/intel-sgx/sgx-linux/2.15/distro/centos8.2-server/
$ ./sgx_linux_x64_sdk_${version}.bin
NOTE: You need to set up the needed environment variables before compiling your code. To do so, run:

$ source ${sgx-sdk-install-path}/environment

Error: I found there's no environment file in that path. 
Fix: sgx sdk has to be installed using sudo.  

$ sudo ./sgx_linux_x64_sdk_${version}.bin

Write no in Do you want to install in current directory? [yes/no]. Then provide /opt/intel/ as the path (surprisingly other custom paths didn't work quite well). After successfully installing set the environment source
$ source /opt/intel/sgxsdk/environment 

Install the sgx driver from. First install the requirements from https://github.com/intel/linux-sgx-driver. To install matching headers 
$ sudo apt-get install linux-headers-$(uname -r)
clea
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

# [Kubeadm](https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/install-kubeadm/)

After installing docker and enabling docker service install kubeadm. Follow this [repo](https://github.com/justmeandopensource/kubernetes/blob/master/docs/install-cluster-centos-7.md) and [Setup Kubernetes Cluster using Kubeadm on CentOS 7](https://www.youtube.com/watch?v=Araf8JYQn3w&t=608s&ab_channel=JustmeandOpensource) for additional info. Reboot the machine if needed.

`sudo swapoff -a` is not persistent across reboot. So either disable swap again after each reboot or try this [solution](https://stackoverflow.com/questions/47094861/error-while-executing-and-initializing-kubeadm)

## Starting the cluster.

```bash
# if using calico networking
sudo kubeadm init --pod-network-cidr=192.168.0.0/16

# else if using flannel networking
sudo kubeadm init --pod-network-cidr=10.244.0.0/16

# else if the network config is already set
sudo kubeadm init

# for the bio-sgx cluster an extra -H was needed for the same command , e.g 
sudo -H kubeadm init --pod-network-cidr=192.168.0.0/16
```

## Error `The connection to the server localhost:8080 was refused`

* Try `journalctl -xeu kubelet` to find the exact cause of kubelet error.
For me it was `misconfiguration: kubelet cgroup driver: \"systemd\" is different from docker cgroup driver: \"cgroupfs\"`.
* The reason is the cgroup needs to be same for both docker and kubernetes. https://kubernetes.io/docs/tasks/administer-cluster/kubeadm/configure-cgroup-driver/
* Solutions from similar issues [1](https://stackoverflow.com/questions/48371610/cluster-install-with-kubeadm-on-ubuntu-failing-on-kubeadm-init-with-getsoc) [2](https://stackoverflow.com/questions/67606941/kubeadm-init-failing-while-initializing-a-kubernetes-cluster).
  * Update following config on all nodes and restart docker and kubelet. The cgroupdriver value can be cgroupfs or systemd. Then restart the services.

    ```bash
      sudo cat << EOF > /etc/docker/daemon.json
      {
        "exec-opts": ["native.cgroupdriver=systemd"]
      }
      EOF

      sudo systemctl restart docker
      sudo systemctl restart kubelet
    ```

* Sometimes the issue can be not having a valid kube config or owning it [1](https://stackoverflow.com/questions/45724889/the-connection-to-the-server-localhost8080-was-refused).

  ```bash
  sudo mkdir -p $HOME/.kube
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

  # or just set it to the default KUBECONFIG location
  export KUBECONFIG=/etc/kubernetes/admin.conf
  chown $(id -u):$(id -g) /etc/kubernetes/admin.conf

  # then restart the service if needed
  sudo service kubelet restart
  ```

* To check if the issue is with config, check config values with the following command and look for null values.

  ```bash
  kubectl config view
  ```

* If persmission denied for config files, change its ownership

  ```bash
  sudo chown $(id -u):$(id -g) /etc/kubernetes/admin.conf

  # then check cluster info again
  kubectl cluster-info
  # Kubernetes control plane is running at https://129.79.240.231:6443
  # CoreDNS is running at https://129.79.240.231:6443/api/v1/namespaces/kube-system/services/kube-dns:dns/proxy

  # To further debug and diagnose cluster problems, use 'kubectl cluster-info dump'.
  ```

## `Your Kubernetes control-plane has initialized successfully!`

  ```bash
  # To start using your cluster, you need to run the following as a regular user:
  mkdir -p $HOME/.kube
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

  # Alternatively, if you are the root user, you can run:
  export KUBECONFIG=/etc/kubernetes/admin.conf
  ```

## Resetting 
To reset your kubeadm cluster by running the reset command and flush your iptables (to avoid any networking issue)
$ kubeadm reset -f && iptables -F && iptables -t nat -F && iptables -t mangle -F && iptables -X
However reseting doesn't clear configs

* does not clean CNI configuration. To do so, you must remove /etc/cni/net.d
* does not clean your kubeconfig files and you must remove them manually
* check the contents of the $HOME/.kube/config file.

Also check https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/troubleshooting-kubeadm/

# Debugging cluster

Found cluster status `not ready` using the following command

```bash
kubectl get nodes --all-namespaces

# NAME        STATUS     ROLES                  AGE     VERSION
# usersrv02   NotReady   control-plane,master   4h54m   v1.22.4

systemctl status kubelet
kubectl describe nodes
```

Found status `container runtime network not ready: NetworkReady=false` and ` reason:NetworkPluginNotReady message:docker: network plugin is not ready: cni config uninitialized`. 

* As said in [here](https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/create-cluster-kubeadm/#pod-network), you must deploy a Container Network Interface (CNI) based Pod network add-on so that your Pods can communicate with each other. Cluster DNS (CoreDNS) will not start up before a network is installed.
* Following soultions from a similar [issue](https://stackoverflow.com/questions/49112336/container-runtime-network-not-ready-cni-config-uninitialized) I have used [Calico](https://docs.projectcalico.org/getting-started/kubernetes/quickstart) :

    ```bash
    # Install the Tigera Calico operator and custom resource definitions.
    kubectl create -f https://docs.projectcalico.org/manifests/tigera-operator.yaml
    kubectl create -f https://docs.projectcalico.org/manifests/custom-resources.yaml

    # Confirm that all of the pods are running with the following command.

    watch kubectl get pods -n calico-system
    # Wait until each pod has the STATUS of Running. 

    # Remove the taints on the master so that you can schedule pods on it.
    kubectl taint nodes --all node-role.kubernetes.io/master-

    # Confirm that you now have a node in your cluster with the following command.
    kubectl get nodes -o wide
    ```

Add pod network add-on. Here calico is used following . You can also have flannel
$ kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/2140ac876ef134e0ed5af15c65e414cf26827915/Documentation/kube-flannel.yml

This got error loading config file "/etc/kubernetes/admin.conf": open /etc/kubernetes/admin.conf: permission denied. Simillar issue https://stackoverflow.com/questions/59179042/kubeadm-upgrade-plan-permission-denied-to-etc-kubernetes-admin-conf. The solution `sudo kubeadm upgrade plan ` faced another error 
[upgrade/health] FATAL: [preflight] Some fatal errors occurred:
        [ERROR ControlPlaneNodesReady]: there are NotReady control-planes in the cluster: [usersrv02 usersrv02]

 Solutions https://github.com/kubernetes/kubeadm/issues/1710. Following didn't work
 $ kubeadm upgrade plan --ignore-preflight-errors=ControlPlaneNodesReady
 Finally it was solved after owning the file for current user
 $ sudo chown $(id -u):$(id -g) /etc/kubernetes/admin.conf

To restart a deployment, get the deployment name first, then .
$ kubectl get deployments
$ kubectl rollout restart deployment tasker-server

https://www.edureka.co/community/31282/is-accessing-kubernetes-dashboard-remotely-possible

When deploying server.yaml the pod kept being at `pending` state. Trying `kubectl get pods -A`, then `kubectl describe pod pod-name` . I found the error was ` 0/1 nodes are available: 1 node(s) had taint {node-role.kubernetes.io/master: }, that the pod didn't tolerate`. Similar issue https://stackoverflow.com/questions/59484509/node-had-taints-that-the-pod-didnt-tolerate-error-when-deploying-to-kubernetes. You can run below command to remove the taint from master node and then you should be able to deploy your pod on that node. You can also get this "taint" type of message when your docker environment doesn't have enough resources allocated.

```bash
kubectl taint nodes  usersrv02 node-role.kubernetes.io/master-
# or
kubectl taint nodes  bio-sgx02 node-role.kubernetes.io/master-
```

https://www.techrunnr.com/how-to-reset-kubernetes-cluster/

# After deploying server service and pods
Now attempt to deploy sec-server faced error ` CrashLoopBackOff` (found using kubectl describe pod pod-name). The exact cause logs in given by command `kubectl logs --follow pod-name`. The sgx-tasks build fails with log 

[build_image /root/linux-sgx/psw/urts/loader.cpp:574] init_enclave failed
[build_secs /root/linux-sgx/psw/urts/loader.cpp:516] Enclave start addr. = 0x7f3880000000, Size = 0x80000000, 2097152 KB
Info: Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards.
Error: Invalid SGX device.


Now to check the error location visit the docker file at `Hysecflow/sgx-tasts/applications/genomeseq/Dockerfile`. Trying to run the same commands in my remote server.
$ sudo yum install libfmt-dev libzmq3-dev libspdlog-dev cmake make git build-essential nano iputils-ping net-tools valgrind
Failed with `Error: Unable to find a match: libfmt-dev libzmq3-dev libspdlog-dev build-essential iputils-ping`.

RUN /usr/bin/cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_C_COMPILER:FILEPATH=/bin/gcc -H$PWD -B$PWD/build -G "Unix Makefiles"



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
For example, ssh -L 12345:localhost:8001 mi3se@usersrv02 -J mi3se@portal.cs.virginia.edu

The dashboard was then available at this url on my pc:

http://localhost:12345/api/v1/kube-system/services/https:kubernetes-dashboard:/proxy/

In terminal opened in vs code editor execute
$ kubectl apply -f .\applications\genomeseq\kubernetes\server.yaml

This will probably fail because of missing args. It is in the file, but for some bug doesn't get loaded. Open minikube dashboard GUI in the web brower.

After getting the GUI, goto deployments and edit server. Then add the missing args (args: ["cd /sgx-tasks/ && git pull && cd /sgx-tasks/build && make && /sgx-tasks/build/applications/genomeseq/server"]) after the command (approx line 160-163), then restart deployment. 

Only after server succeeds deploy worker nodes.

```bash
# check deployed pods
kubectl get pods --all-namespaces

# to check whether things are running ok
kubectl cluster-info

# get the list of pods and then log into the server pod using server pod name
kubectl exec --stdin --tty tasker-server-6fd69d8888-4l5lm -- /bin/bash

kubectl exec --stdin --tty tasker-server-d5bf4b88c-zrr9v -- /bin/bash

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
# download client genome from https://www.internationalgenome.org/data-portal/search?q=SRR062634
ls -lah /data/

# Now log into client again . Then partition and index the genome file. Here we are creating 4 partitions.
./client client1
client$ index -p 2 -s hg38.fa

# Check log of client pod to see how it partitioned the data. 
# If we scale up the worker node to have multiple nodes, they'll pickup the other partitions to index

# after the partitioning is done check the index ID
client$ ls 

# then run the aligning. this will be done by the secure worker
client$ mem -s SRR062634.filt.fastq -i index_7zdl337n

```
We also can follow pod logs in real time
$ kubectl logs --follow podName
