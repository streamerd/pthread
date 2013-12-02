## A practice about backoff algorithm about avoiding deadlock

**two thread, three mutexes**

* first thread locks the mutexes in a order and the other locks them in a reverse order

* use backoff algorithm  to avoid deadlock

**yield flag and backoff flag**

* you can set up yield flag to force the two thread being concurrency

* you can set up backoff flag to use or not use backoff algorithm

