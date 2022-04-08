Problem Statement for the code:
Write TCP client to download and locally save a file.
1. The client, as a command-line argument, accepts the URL of the file. The URL can be HTTP or HTTPS-based.
2. If the URL does not exist or unreachable, the client prints an error and exits.
3. After connecting to the server, the client sends a GET request to the server to get the file.
4. The client downloads the whole file and saves it to the local folder with the same file name as mentioned in the URL.
5. If a file with the same name exists, the old file is overwritten.
6. The client exits after downloading the file.





To compile the TCPclient file, use the following command:

gcc TCPclient.c -lssl -lcrypto -o c.out



To download any file from the internet, use the following command:

./c.out [URL]

[URL] is the full URL of the desired file to be downloaded.


