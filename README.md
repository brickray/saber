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

tb = {} //create a table
tb.fib = fib //fibonacci
tb.fibp = 10
tb.iter = def (key, value) print(key, value) end //table iteration function

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

def func()
    print("co-body1")
    coroutine.yield()
    print("co-body2")
end

co = coroutine.create(func)
coroutine.resume(co)
print("resume1")
coroutine.resume(co)
print("resume2")


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
--co-body1
--resume1
--co-body2
--resume2
```
