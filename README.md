# SOI - Monitor z użyciem C++ i Boost

# Uruchamianie

```
git clone https://github.com/mRokita/SOI_Semaphores.git
git checkout monitor
mkdir build
cd build
cmake ..
make
./soi3
```

# Krótka dokumentacja

Główna część programu znajduje się w nagłówku `Buffer.h`

## Sekcja krytyczna

Główny element programu to metody `Buffer::push(int)` oraz `Buffer::pop()`  
Obie po dodaniu / usunięciu elementu z kolejki wywołują metodę `Buffer::processConditions()`, która zajmuje się otwieraniem odpowiednich semaforów w zależności od stanu kolejki, na podstawie pól `Buffer::oddCount` oraz `Buffer::evenCount`.

```C++
void push(short num){
    if(!alive) return;
    queue[end] = num;
    end ++;
    num % 2 ? oddCount ++ : evenCount ++;
    if(end >= QUEUE_MAX_SIZE){
        std::cerr << "QUEUE MAX SIZE EXCEEDED" << std::endl;
        destroy();
    }
    processConditions();
}
```


```C++
void pop(short odd){
    if(!alive) return;
    if(queue[begin] % 2 == odd){
        begin ++;
        odd == 1 ? oddCount -- : evenCount --;
    }
    processConditions();
}
```
## Akcje poszczególnych procesów

Dla każdego procesu jest dedykowana metoda implementująca akcję charakterystyczną dla danego procesu. 
Metody te są skonstruowane w podobny sposób, najpierw alokowany jest zamek, następnie, gdy zamek zostanie zwolniony, sprawdzany jest warunek charakterystyczny dla procesu. Jeśli warunek nie jest spełniony, następuje oczekiwanie realizowane przez `interproces_condition::wait` wykonywane na odpowiednim warunku.
funkcja `interproces_condition::wait` najpierw zwalnia dany zamek, następnie blokuje wątek lub proces i oczekuje na wywołanie `interproces_condition::notify_one` lub `interproces_condition::notify_all` w innym procesie lub wątku.

```C++
void pushEven(short num){
    scoped_lock<interprocess_mutex> lock(mutex);
    if (evenCount >= 10) {
        A1Condition.wait(lock);
    }
    push(num);
}

void pushOdd(short num){
    scoped_lock<interprocess_mutex> lock(mutex);
    if (evenCount >= oddCount){
        A2Condition.wait(lock);
    }
    push(num);
}

void popEven(){
    scoped_lock<interprocess_mutex> lock(mutex);
    if (evenCount < 3) {
        B1Condition.wait(lock);
    }
    pop(0);
}

void popOdd(){
    scoped_lock<interprocess_mutex> lock(mutex);
    if (oddCount < 7){
        B2Condition.wait(lock);
    }
    pop(1);
}
```

## Metoda `processConditions` 

Metoda ta jest wywoływana po każdej operacji zmieniającej zawartość kolejki. Informuje następny procesy lub wątki o spełnieniu konkretnego warunku, wykorzystując `interproces_condition::notify_one`.

```C++
void processConditions(){
    if (evenCount < 10) A1Condition.notify_one();
    if (oddCount < evenCount) A2Condition.notify_one();
    if (evenCount >= 3) B1Condition.notify_one();
    if (oddCount >= 7) B2Condition.notify_one();
}
```

## `Buffer::getInstance`

Klasa `Buffer` jest implementacją wzorca programowania obiektowego _Singleton_.  Parametr `ref` służy do określenia, czy zwrócona referencja ma zostać policzona.
Jeśli `ref == true`, wartość `Buffer::references` jest zwększana o `1`.

```C++
static Buffer* getInstance(bool ref = true){
    std::pair<Buffer *, std::size_t> sharedMemory = segment.find<Buffer>(unique_instance);
    Buffer* inst;
    if(sharedMemory.first == nullptr){
        std::cout << "Init buffer..." << std::endl;
        inst = segment.construct<Buffer>(unique_instance)();
    } else {
        inst = sharedMemory.first;
    }
    if (ref) inst->references ++;
    return inst;
}
```

## Dealokacja pamięci

Do dealokacji pamięci służy metoda `Buffer::cleanup`.

Zmniejsza ona `Buffer::references` o `1`

Następnie, jeśli `Buffer::references == 0`, dealokuje segment pamięci współdzielonej, zwalnia mutex oraz informuje o spełnieniu warunków. Spełnienie warunków nie powoduje jednak wykonania operacji, ponieważ wcześniej do `Buffer::alive` przypisana jest wartość `false`.

Dużo lepszym rozwiązaniem byłoby wykorzystanie wzorca projektowego RAII, ponieważ takie podejście mogłoby się okazać problematyczne gdyby projekt był dalej rozwijany.

```C++
void cleanup(){
    alive = false;
    references --;
    if(references == 0 ) {
        std::cout << "Received SIGINT..." << std::endl;
        shared_memory_object::remove(SEGMENT_NAME);
        mutex.unlock();
        A1Condition.notify_all();
        A2Condition.notify_all();
        B1Condition.notify_all();
        B2Condition.notify_all();
        std::cout << "Destroyed shared memory segment" << std::endl;
    }
}
```