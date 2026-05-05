// ============================================================
// SAMPLE 1 — Hello World
// ============================================================
let msg string = "Hello, World!";
print(msg);


// ============================================================
// SAMPLE 2 — Basic Arithmetic
// ============================================================
let a int = 10;
let b int = 3;
let sum     int = a + b;
let diff    int = a - b;
let product int = a * b;

print(sum);
print(diff);
print(product);


// ============================================================
// SAMPLE 3 — Float Arithmetic & Mixed Print
// ============================================================
let pi    float = 3.14159;
let r     float = 5.0;
let area  float = pi * r * r;
let circ  float = 2.0 * pi * r;

print(area);
print(circ);


// ============================================================
// SAMPLE 4 — Reassignment & Running Total
// ============================================================
let total int = 0;
total = total + 10;
total = total + 25;
total = total + 5;
print(total);          // 40


// ============================================================
// SAMPLE 5 — Multi-arg Print
// ============================================================
let x int = 7;
let y int = 8;
let z int = x * y;
print(x, y, z);        // 7 8 56


// ============================================================
// SAMPLE 6 — Fibonacci (unrolled, 8 terms)
// ============================================================
let f0 int = 0;
let f1 int = 1;
let f2 int = f0 + f1;
let f3 int = f1 + f2;
let f4 int = f2 + f3;
let f5 int = f3 + f4;
let f6 int = f4 + f5;
let f7 int = f5 + f6;

print(f0, f1, f2, f3, f4, f5, f6, f7);


// ============================================================
// SAMPLE 7 — Compound Expressions & Precedence
// ============================================================
let val int = 2 + 3 * 4;        // 14  (not 20)
let val2 int = (2 + 3) * 4;     // 20
print(val, val2);


// ============================================================
// SAMPLE 8 — String variables and labels
// ============================================================
let label1 string = "Area:";
let label2 string = "Circumference:";
let radius float = 7.0;
let a2 float = 3.14159 * radius * radius;
let c2 float = 2.0 * 3.14159 * radius;
print(label1, a2);
print(label2, c2);
