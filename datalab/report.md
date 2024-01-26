# 实验目的

# 实验过程
## 开始
执行`make btest`报错：`fatal error: bits/libc-header-start.h: No such file or directory`，于是执行如下命令安装缺少的库：
```bash
$ sudo apt-get install gcc-multilib
```
之后再执行`make btest`即可以正常编译。
## bitXor
由逻辑代数的知识，有$$A\oplus B=\overline{A}B+A\overline{B}$$
而我们不能使用或运算，故可以使用德摩根定律将其展开：
$$X+Y=\overline{\overline{X+Y}}=\overline{\overline{X}\ \overline{Y}}$$
代入异或的运算式即可，最终代码为：
```c
int bitXor(int x, int y) {
  return ~(~(~x & y) & ~(x & ~y));
}
```
测试如下：
