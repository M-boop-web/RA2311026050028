// Sample DSL program
// Variable declarations with types

let x int = 10;
let y int = 20;
let z float = 3.14;
let msg string = "Hello from DSL!";

// Arithmetic
let sum int = x + y;
let product int = x * y;
let combined float = z * 2;

// Reassignment
x = x + 5;

// Print statements
print(msg);
print(sum);
print(product);
print(combined);
print(x, y, sum);
