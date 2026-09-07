# GUI-Mac-Changer

This tool changes the MAC address and features a graphical interface; you can enter a specific MAC address—such as 00:11:22:33:44:55—or use the random option to select a random MAC address.

## The Tool by - Yen's

##Instalation

## if you want to run the tool in the same dirctory use this commands

```bash
sudo apt update
```
```bash
sudo apt install gcc -y 
```
```bash
sudo apt install -y gcc make pkg-config libgtk-3-dev libglib2.0-dev
```
```bash
git clone https://github.com/vnun0-yo/GUI-Mac-Changer.git
```
```bash
cd GUI-Mac-Changer
```
```bash
sudo gcc -o YenGuiMacChanger YenGuiMacChanger.c $(pkg-config --cflags --libs gtk+-3.0) -lpthread
```

```bash
sudo ./YenGuiMacChanger
```

##if you want to run the tool from anywhere in the system

```bash
sudo apt update
```
```bash
sudo apt install gcc -y 
```
```bash
sudo apt install -y gcc make pkg-config libgtk-3-dev libglib2.0-dev
```
```bash
git clone https://github.com/vnun0-yo/GUI-Mac-Changer.git
```

```bash
cd GUI-Mac-Changer
```
```bash
sudo cp YenGuiMacChanger /usr/bin/YenGuiMacChanger
```
```bash
sudo chmod +x /usr/bin/YenGuiMacChanger
```

## The Tool By - Yen's Enjoy!
