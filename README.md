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

После установки будут размещены:

- бинарник: `/usr/bin/staff-control`
- конфиг(Настройки): `/etc/staff-control/cfg.json`
- unit-файл systemd: `/usr/lib/systemd/system/staff-control.service`
- БД: `/tmp/staff-control/staff-control.service`


### Параметры(cfg.json)

````json
{
  "port": 8080,
  "log_level": 6,
  "path_db": "/tmp/staff-control/staff-control.json",
  "targets": [
    {
      "name": "kassa1",
      "path": "/tmp/kassa1",
      "file_name": "Pos01.spr",
      "flag_name": "Pos01.flz"
    },
    {
      "name": "kassa2",
      "path": "/tmp/kassa2",
      "file_name": "Pos02.spr",
      "flag_name": "Pos02.flz"
    }
  ]
}
````

- `port` — порт, на котором запускается HTTP-сервер
- `log_level` — уровень логирования для `syslog`
- `path_db` — путь до БД
- `targets` — цели (кассы), куда будут выгружаться файлы с пользователями
    - `name` — имя кассы
    - `path` — путь к директории кассы
    - `file_name` — имя файла выгрузки
    - `flag_name` — имя флага выгрузки
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
