# Configuration mode

This mode is used to edit or upload profiles via ftp.

Profile 10 is reserved for **Configuration mode**. It is not necessary to upload **profile10.json**. 

In this mode, controller is creating WIFI access point with FTP server.

It is possible to change **Configuration mode** default properties by uploading **profile10.json**.


**profile10.json** example:
```json
{
    "transmitter_mode" : "CONFIG",
    "ap_name" : "hxrct",
    "ap_password" : "",
    "ftp_user" : "anonymous",
    "ftp_password" : "anonymous"
}

```

As always, you can also upload whole filesystem with PlatformIO.
