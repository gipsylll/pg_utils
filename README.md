## run
```bash
mkdir build && cd build
cmake .. 
make
cd ..
sudo chmod 777 ./backups
docekr compose up -d
python3 test.py
```


