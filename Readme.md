# barrier-free elevator system

The concept of this project is to design a specialized elevator that is friendly to individuals with mobility impairments. Users can indicate their intention to go up or down by giving a thumbs-up or thumbs-down gesture, respectively. The elevator will prioritize service for these individuals.

---

### system structure

![截圖 2024-06-08 下午4.02.13.png](barrier-free%20elevator%20system%2072afa05f4eae413db0f4037bdfcfafcc/%25E6%2588%25AA%25E5%259C%2596_2024-06-08_%25E4%25B8%258B%25E5%258D%25884.02.13.png)

---

### manual

compile:  `make` 

1. Use text files as input to provide developers with the ability to detect algorithms

`./elevator_system` 

`./txt_input <pid> <file>`

1. actual use

server :

`./elevator_system` 

`./server <pid>  <port>`

client:

normal user

`./client <ip> <port>`

`<name> <type> <floor you are now>  <destination> <direc up/down>`

disable person

`./python3 [mymediapipe.py](http://mymediapipe.py)`  you can change “name”,”type”,”floor you are now”,”destination” in this file 

After opening the lens, the thumb up or down represent go up or down

---

### Demo