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

print(fib(7))
```
