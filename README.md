# Saber
saber is a toy script language

# Example
```
def fib(a)
    if a == 1 || a == 0 then
        return 1
    end

    return fib(a - 1) + fib(a - 2)
end

tb = {
      "fib" = fib, 
      "fibp" = 10, 
      "iter" = def(key, value) print(key, value) end
     } //create a table

print(tb.fib(tb.fibp), table.len(tb))
table.foreach(tb, tb.iter) //traverse table

def Test(n1, ...)
    local a = "test"
    print(string.format("%s, %s", a, n1));
    local i
    for i = 0, i < args.num, i += 1 do
	print(args[i])
    end
end

Test("a1", "a2", "a3", 1, 2, 3, 4)
Test("c1", "c2", 2, 3)


def send(p)
    print("product send : ", p)
    coroutine.yield(p)
end

def func()
    local p
    while true do
        p = io.input()
	send(p)
    end
end
local product = coroutine.create(func)

def receive(co)
    local res = coroutine.resume(co)
    if res then
	print("consumer received : ", res)
    else
	print("consumer receive failed : ", res)
    end
end

def consumer(co)
    while true do
	receive(co)
    end
end
consumer(product)


//result
--89  3
--fib  function
--iter  function
--fibp  10
--test, a1
--a2
--a3
--1
--2
--3
--4
--test, c1
--c2
--2
--3
```
