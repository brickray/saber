# Saber
saber is a toy script language

# Example
```
def fib(a)
	if a == 1 || a == 0 then
		return 1
	end

	return fib(a - 1) + fib(a-2)
end

tb = {} //create a table
tb.fib = fib //fibonacci
tb.fibp = 10
tb.iter = def (key, value) print(key, value) end //table iteration function

print(tb.fib(tb.fibp), table.len(tb))
table.foreach(tb, tb.iter) //traverse table


//result
--89  3
--fibp  10
--iter  function
--fib  function
```
