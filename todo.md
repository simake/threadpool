* car-building example (from many small parts to few medium parts to one big part)
* use promise instead of packaged_task (to try to circumvent packaged_task not being copyable)
* benchmark ThreadPool vs std::thread, etc.
* stop or join? vad ska man kunna göra? vad är automatiskt? terminate?
* what happens if queue is cleared and there's a future left? (what does future.get() do? future.valid()?)
* optionally use priority (priority queue)
* reach pool using global static function

* lever packaged_task tills future.get()? blockar det tråden, eller kan den köra vidare utan future.get()?
    * testa, men tror inte att det blockar. future och promise har shared state, så även om promise (i packaged_task) försvinner kommer future att hålla resultatet levande.

readme:
* what is this? why?
* relies heavily upon threading and synchronization classes from the C++11 Standard Library (but I have deliberately avoided features that are deprecated in later versions)
* create basic example(s) in README.md

