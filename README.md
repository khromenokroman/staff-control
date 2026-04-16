# Staff Control

Веб-приложение для управления сотрудниками кассовых рабочих мест.

Программа позволяет через браузер:
- просматривать список сотрудников;
- добавлять новых сотрудников;
- удалять сотрудников;
- выгружать данные в файлы для касс.

Данные сотрудников хранятся в JSON-файле, а управление выполняется через встроенный HTTP-сервер.


## Требования

### Для сборки

- CMake 3.26+
- C++20
- `fmt`
- `cpp-httplib`
- `nlohmann-json`

````bash
apt install -y build-essential cmake libfmt-dev nlohmann-json3-dev dpkg-dev libcpp-httplib-dev
````

## Сборка из исходников

```bash 
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
cmake --build . -j$(nproc)
cmake --install .
```

## Сборка DEB

Пакет собирается через CPack:

```bash 
cd build cpack -G DEB
apt install -y ./staff-control_<версия>_amd64.deb
```

## Запуск

После установки сервис можно запускать так:

```bash 
systemctl daemon-reload
systemctl enable staff-control
systemctl start staff-control
```

Проверка статуса:

```bash 
systemctl status staff-control
```

По умолчанию сервер доступен на: http://localhost:8080
