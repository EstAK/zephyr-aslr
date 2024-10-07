# how to use the fast & easy scripts
1. run `create_and_flash.sh`
2. put the device into maskrom mode
3. press enter
4. paste into minicom and press enter
5. coppy from below `*** Booting Zephyr OS build <whateverid> ***` and paste into a file
6. repeat from step 2

## example
you should now have two files each looking similarly to the following,
```txt
ASLR disabled
no caches
user2_thread yields at 173306421
user1_thread is back 173896472

    .
    .
    .

user1_thread took over at 173902079
user2_thread is back 173909030
user2_thread took over at 173914588
```
# interpret results
use the python script,
```shell
python read_results.py file1 file2 filen
```
this will output csv files for each file in the format (from, to, time)

> you have to keep track of which file has the aslr enabled

also, you can change the behavior of the scripts such that it won't generate CSV files by passing
the argument `-no-csv` as the **first argument**
