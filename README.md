# Password-Recovery-with-Threads
This project showcases a concurrent programming approach to recover passwords by exploiting the power of multithreading and efficient task management. Utilizing a custom-built FIFO queue for task distribution, the system efficiently cracks password hashes using a thread pool.

# Key Features
Multithreading for Efficiency: Leverages multithreading to enhance the speed of password recovery, crucial for processing large sets of data.
FIFO Queue Implementation: Utilizes a First In, First Out (FIFO) queue for orderly task management and distribution to worker threads.
Thread Pool Management: Employs a thread pool to optimize resource usage by reusing threads for multiple tasks, reducing the overhead of thread creation and destruction.
Google Compute Engine Utilization: Executes the password recovery process on the Google Compute Engine, taking advantage of its robust infrastructure to handle intensive computational tasks.

# Technical Implementation
Concurrent Data Structures: Designed and implemented thread-safe concurrent data structures, including the core FIFO queue.
Efficient Task Execution: Developed a mechanism for worker threads in the pool to retrieve and execute tasks from the queue, ensuring efficient workload distribution.
Password Hash Cracking: Implemented a client for the thread pool that attempts to brute-force passwords from their hashes. Each task consists of trying different permutations of dictionary words augmented with digits.
Robustness and Race-Free Code: Emphasized writing race-free code that works consistently under concurrent execution scenarios, avoiding common pitfalls like race conditions.

# Application
This system can be applied to scenarios requiring efficient parallel processing of tasks, especially where data processing and computational tasks are extensive and time-sensitive.
