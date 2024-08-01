# Formatting Output

## Formatting tasks
The `task` type is formattable in the okec library. Consequently, it can be directly formatted in both the log module and the output module using the syntax `{:t}`, as demonstrated in the following example:

```cpp
okec::print("{:t}", t);
olog::info("{:t}", t);
```