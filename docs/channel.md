# channel

channel是由Hoare的CSP派生的同步原语之一.虽然它们可以用来同步内存访问,但它们最好用在coroutine之间传递信息.  

coroutine协程库参考golang中的channel也提供类似的机制方便用户编写并发程序.