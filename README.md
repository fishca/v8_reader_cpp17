# v8_reader_cpp17

Кроссплатформенный просмотрщик конфигураций 1С, написанный на C++17 без зависимостей от платформы 1С.

## 🏗️ Архитектура
Проект разделён на 3 независимых слоя:
- `core` — чистое C++17, парсинг `.1CD`/`.cf`, работа с памятью и сжатием (zlib). **0% зависимостей от UI**.
- `ui` — Qt6 компоненты. Зависит только от интерфейсов `core`.
- `app` — точка входа, композиция зависимостей.

## 🛠️ Сборка (Windows + Visual Studio)

### 1. Установка зависимостей
```powershell
# Установите vcpkg: https://github.com/microsoft/vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Установите пакеты
.\vcpkg install qt6-base qt6-tools zlib --triplet=x64-windows