# AGENTS.md — Інструкції та контекст для AI-агентів у проєкті FlymasterDump

## 1. Огляд проєкту

**FlymasterDump** — це десктопний кросплатформний C++ застосунок з графічним інтерфейсом на базі Qt 5.
Призначення: зчитування списку польотів із варіометра Flymaster (GPS SD Vario) через послідовний порт (Serial / USB-UART) та вивантаження треків у формат IGC (`.igc`).

### Технологічний стек:
- **Мова:** C++14
- **GUI & Core:** Qt 5 (Widgets, Core, Gui)
- **Мережа/Асинхронний I/O:** Boost.Asio (включно з `boost_system`, `boost_date_time`)
- **Тестування:** Google Test (GTest)
- **Система збірки:** CMake (3.10+)
- **Середовище та пакети:** Підтримка Homebrew (macOS) та Nix (Flakes, `nix-shell`)

---

## 2. Мовні та комунікаційні правила

- **Внутрішній аналіз і процес мислення (Chain of Thought):** Завжди вести виключно українською мовою.
- **Відповіді та діалог з користувачем:** Завжди формулювати українською мовою.
- **Код та ідентифікатори:** Зберігати оригінальний синтаксис (назви класів, функцій, змінних, файлів, прапорців компілятора), але роздуми та коментарі подавати українською мовою.

---

## 3. Архітектура та структура кодової бази

```
FlymasterDump/
├── CMakeLists.txt              # Головний конфігураційний файл CMake
├── flake.nix / shell.nix       # Опис залежностей та оточення для Nix / Nix Flakes
├── scripts/
│   └── thirdparty.cmake        # Конфігурація сторонніх бібліотек (Qt5, Boost, GTest)
├── progress/                   # Бібліотека звітування прогресу (progressLib)
│   ├── Callback.h / .cpp       # Базовий інтерфейс зворотного виклику та Severity рівні
│   └── Callback.test.cpp       # Unit-тести для progressLib
├── serialPortCom/              # Бібліотека зв'язку та протоколу Flymaster (serialPortComLib)
│   ├── SerialPort.h / .cpp     # Робота з COM / serial портом на базі Boost.Asio
│   ├── Request.h / .cpp        # Високорівневі запити (серійний номер, список, завантаження)
│   ├── IgcFile.h / .cpp        # Структура файлу IGC
│   ├── IgcFactory.h / .cpp     # Конвертація бінарних пакетів Flymaster у формат IGC
│   ├── ascii/                  # ASCII/NMEA протокол Flymaster ($PFMSNP, $PFMDNL тощо)
│   │   ├── FlightMetaInfo.*    # Метадані окремого польоту
│   │   ├── NmeaSentence.*      # Парсинг та підрахунок контрольної суми NMEA
│   │   └── StringUtils.*       # Утиліти розбиття та об'єднання рядків
│   └── binary/                 # Бінарний протокол передачі даних польоту
│       ├── BinaryBlock.*       # Базовий бінарний блок з CRC
│       ├── Convert.*           # Little-endian конвертація чисел
│       ├── FlightInformationRecord.* # Запис інформації про прилад
│       ├── KeyTrackPositionRecord.*  # Базовий запис точки GPS треку
│       └── TrackPositionRecordDeltas.*# Дельта-стиснуті точки треку
└── app/                        # Головний виконуваний застосунок (variodump) та UI (varioDumpUiLib)
    ├── main.cpp                # Точка входу QApplication
    ├── MainWindow.h / .cpp / .ui # Головне вікно та елементи керування
    ├── Model.h / .cpp          # Модель параметрів (порт, швидкість, метадані пілота)
    ├── Property.h              # Реактивна обгортка властивостей із Qt-сигналами
    ├── QtCallback.h / .cpp     # Міст між Task callback та Qt подіями/прогрес-баром
    ├── TaskProcessor.h / .cpp  # Асинхронний диспетчер задач у фоновому потоці
    ├── TaskProgressDialog.*    # Модальне вікно відображення прогресу задачі
    ├── CheckDeviceTask.*       # Задача перевірки підключення та отримання інфо про прилад
    ├── FlightListTask.*        # Задача зчитування списку польотів
    ├── DumpTask.*              # Задача вивантаження треку та збереження в .igc
    └── TaskProcessor.test.cpp  # Тести асинхронного процесора задач
```

---

## 4. Збірка, запуск і тестування

### 4.1. Збірка через CMake (macOS / Linux)
За наявності встановлених бібліотек (наприклад, через Homebrew):
```bash
# Конфігурація з Apple Clang або системним компілятором
cmake -B build \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@5;/opt/homebrew/opt/boost;/opt/homebrew/opt/googletest"

# Компіляція
cmake --build build -j

# Запуск усіх тестів
ctest --test-dir build --output-on-failure
```

### 4.2. Робота через Nix
```bash
# Збірка пакету
nix build

# Або запуск оточення розробки з усіма залежностями
nix develop
# або
nix-shell
```

### 4.3. Запуск застосунку
Виконуваний файл генерується за адресою:
```bash
./build/app/variodump
```
Для перевірки без відкриття GUI-вікна (headless):
```bash
./build/app/variodump -platform offscreen
```

---

## 5. Важливі правила розробки та потенційні пастки (Gotchas)

1. **Кросплатформність (уникати MSVC/Windows-специфічного коду):**
   - Не використовувати MSVC функції на кшталт `_itoa_s`, `_putenv`, `sprintf_s`. Замість них використовувати стандартизовані C++ функції (`snprintf`, `setenv` з `tzset()` на POSIX).
   - Прапорці компонувальника MSVC (наприклад `/ENTRY:mainCRTStartup` або `_WIN32_WINNT`) повинні бути ізольовані під `if(MSVC)` або `if(WIN32)`.
   - Регістр імен файлів: заголовки повинні точно відповідати імені файлу (наприклад, `#include "TaskProcessor.h"`, а не `"taskProcessor.h"`).

2. **Сучасний Boost.Asio:**
   - Використовувати `boost::asio::io_context` замість застарілого `io_service`.
   - Використовувати `boost::asio::steady_timer` та `std::chrono::milliseconds` замість застарілого `deadline_timer` та `posix_time`.
   - Надавати перевагу лямбда-функціям C++11/14 замість `boost::bind` (запобігає конфліктам з `_1` та застарілими макросами).

3. **Специфіка послідовних портів:**
   - На macOS послідовні порти зазвичай мають вигляд `/dev/cu.usbserial*` або `/dev/cu.SLAB_USBtoUART`.
   - Комбобокс вибору порту в UI має залишатися редагованим (`setEditable(true)`), що дає можливість підключатися до PTY/віртуальних портів або пристроїв, підключених після запуску.

4. **Тести GoogleTest:**
   - Тести, що використовують класи `QObject` або реєструють типи в мета-системі Qt (як-от `variodumpTest`), обов'язково повинні ініціалізувати `QCoreApplication app(argc, argv);` у функції `main()`.
   - У `CMakeLists.txt` для `add_test(NAME ... COMMAND ...)` завжди передавати ім'я CMake-таргету, а не захардкоджений відносний шлях.

5. **Форматування коду:**
   - Дотримуватися стилю, визначеного в кореневому [`.clang-format`](file:///Volumes/SRC/FlymasterDump/.clang-format).
