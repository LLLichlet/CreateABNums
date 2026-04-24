CreateABNums
A simple calculator for guidance to craft nums 0-9 in _Create: Above and Beyond_.

Robustness is not guaranteed. Results for reference only.

Licensed under Public Domain. Feel free to use anywhere.

Expected Results:

--------------------
sum cost = 32
atomic:
0 = (3 - 3)
1 = (3 / 3)
2 = (3 - (3 / 3))
3 = 3
4 = ((3 / 3) + 3)
5 = (8 - 3)
6 = (3 + 3)
7 = (8 - (3 / 3))
8 = 8
9 = (3 * 3)
--------------------

operators in total: 

--------------------
3: 18
8: 3
+: 2
-: 4
*: 1
/: 4
--------------------

Corollary
Since this is proved to be the minimum cost, minor manual adjustments can be done to make 3 and 8 consumed relatively evenly:

--------------------
0 = (3 - 3)
1 = (8 / 8)
2 = (3 - (8 / 8))
3 = 3
4 = ((8 / 8) + 3)
5 = (8 - 3)
6 = (3 + 3)
7 = (8 - (8 / 8))
8 = 8
9 = (3 * 3)
--------------------

Thus the cost is corrected to {3: 10, 8: 11, ...}.
