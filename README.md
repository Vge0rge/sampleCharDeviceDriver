# Sample char device driver
This is a simple char device driver that I wrote and I wanted to publish here mostly for education purposes. This simple driver has a device that accepts messages and prints them when asked one by one. Each message has a limit and there is also a global limit of all messages combined. 


## Usage 
```bash 
make 
insmod charDeviceDriver.ko
mknod /dev/whaterverTheSysLogSays
cat "testMessage" > /dev/opsysmem
cat /dev/opsysmem
```

## License

This project is licensed under the MIT License - see the [LICENCE.md](LICENCE.md) file for details
