## Day 1 – ESP-IDF Setup
---
**Resources I followed:**  
- Official guide: [ESP-IDF Windows Setup](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html#get-started-windows-first-steps)  
- The forum thread where someone had the same problem I encountered: [ESP32 Forum](https://esp32.com/viewtopic.php?t=13392)  
- Solution: download IDF from [GitHub Releases](https://github.com/espressif/esp-idf/releases/)  
- First project flashed to the ESP board: [hello_world example](https://github.com/espressif/esp-idf/tree/v5.5.1/examples/get-started/hello_world)
---

I opened up cmd (Win+R) and went straight to downloading the online installer from Espressif. (No, I didn’t do it via cmd — just wanted to feel cool while downloading from Microsoft Edge 😎)

I ran the installer… and midway, boom. Error.  

Not just once — I reran the installer, same error. Even after deleting and re-downloading it, even after wiping out the `.espressif`. Same stubborn error:

```text
Path = C:\\Espressif\\releases\\esp-idf-v5.5.1.zip
Type = zip
ERRORS:
Unexpected end of archive
Physical Size = 918819453
Characteristics = Local

ERROR: Data Error : esp-idf-v5.5.1\\.git\\modules\\components\\esp_wifi\\lib\\objects\\pack\\pack-cc8c78da0ac83e1bbb1c921824460305418cc036.pack

Sub items Errors: 1

Archives with Errors: 1

Open Errors: 1

Sub items Errors: 1
```

Looked like a corrupted zip file to me. I followed the advice of someone online and went straight to GitHub:
👉 [GitHub Releases](https://github.com/espressif/esp-idf/releases/)

Their README recommended cloning instead of downloading the archive, so that’s exactly what I did:
```bash
git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-v5.5.1
cd esp-idf-v5.5.1/
```

The download took a while (15… maybe 30 minutes? Honestly I zoned out and lost track).
From the documentation, I understood it is recommended to download it into this directory:
```bash
%userprofile%\Desktop\esp-idf
```
Correct me if i m wrong, i downloaded it there.

So I put it there and re-ran the installation file i dowladed in the first step (they don't have to be in the same directory). This time, the installer finished and gave me this nice little PowerShell window:
![attachment:c76a6c14-2e42-4362-af0b-10ef59e33b0e:image.png](https://github.com/TamaGo-HQ/espidf_get-started/blob/main/images/success_install.png)
Closed it. Back to cmd.

The GitHub repo mentioned example projects to get started, so I went hunting inside the ESP-IDF folder until I found the examples/get-started directory. And there it was: hello_world!

Time to actually test my board. I plugged it in and ran the mode command to see which COM port it was on:
```bash
Output:

Status for device COM9:

Baud:            115200
Parity:          None
Data Bits:       8
Stop Bits:       1
Timeout:         OFF
XON/XOFF:        OFF
CTS handshaking: OFF
DSR handshaking: OFF
DSR sensitivity: OFF
DTR circuit:     OFF
RTS circuit:     OFF
```

Cool, board is on COM9.

Then I tried:
```bash
idf.py set-target esp32
```

But cmd hit me back with:
```bash
'idf.py' is not recognized as an internal or external command,
operable program or batch file.
```

So… back to the ESP-IDF root directory. Ran:
```bash
install.bat
```

Output confirmed it was using the correct IDF path. Then ran:
```bash
export.bat
```

And finally tested it with:
```bash
idf.py --version
```

Result:
```bash
ESP-IDF v5.5.1
```
You can follow the steps on this page: [ESP-IDF Windows Setup](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html#get-started-windows-first-steps) to build and run the **hello_world** program.

Two problems I encountered along the way:

- **Configuration window:** Not sure what to do when it popped up? Answer: nothing 😅 Unless you have a board with a single core, the instructions tell you what to do. I have a dual-core ESP32, so I just went with the default and pressed `Esc`.  

- **Exiting the IDF monitor on an AZERTY keyboard:** Still a bit tricky… I ended up switching to a QWERTY layout, looked up where the `]` key is, and pressed `Ctrl + ]` to exit.  

🎉 Success! My ESP-IDF is now up and running.

⚠️ **Important note:** `export.bat` only works for the **current terminal session**. If you open a new cmd window and want to run `idf.py`, you need to run `export.bat` again (unless you permanently add it to your system `PATH`).
