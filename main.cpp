#include <fstream>
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

const std::string JSON_FILE = "/tmp/info_user.json";
// const std::string PATH_KASSA1 = "/home/raipo/share/kassa1/";
const std::string PATH_KASSA1 = "/tmp/kassa1/";
// const std::string PATH_KASSA2 = "/home/raipo/share/kassa2/";
const std::string PATH_KASSA2 = "/tmp/kassa2/";

struct UserInfo {
    int password{};
    int pin_card{};
    int role{};
};

using Users = std::unordered_map<std::string, UserInfo>;

int random_int(int from, int to) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(from, to);
    return dis(gen);
}

std::string html_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c: s) {
        switch (c) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            case '\'':
                out += "&#39;";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

Users load_info() {
    Users users;
    std::ifstream f(JSON_FILE);
    if (!f.is_open()) {
        return users;
    }

    json j;
    try {
        f >> j;
        for (auto it = j.begin(); it != j.end(); ++it) {
            UserInfo info;
            info.password = it.value().value("password", 0);
            info.pin_card = it.value().value("pin_card", 0);
            info.role = it.value().value("role", 0);
            users[it.key()] = info;
        }
    } catch (...) {
        return {};
    }

    return users;
}

bool save_info(const Users &users) {
    json j;
    for (const auto &[name, info]: users) {
        j[name] = {{"password", info.password}, {"pin_card", info.pin_card}, {"role", info.role}};
    }

    std::ofstream f(JSON_FILE);
    if (!f.is_open())
        return false;

    f << j.dump(4);
    return true;
}

std::string role_name(int role) {
    if (role == 1)
        return "кассир";
    if (role == 2)
        return "старший кассир";
    return "неизвестно";
}

std::string build_index_page() {
    return R"html(
<!doctype html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Frontol User</title>
    <style>
        :root {
            --bg1: #1f2a44;
            --bg2: #243b55;
            --card: rgba(255, 255, 255, 0.10);
            --card-hover: rgba(255, 255, 255, 0.16);
            --text: #f5f5f5;
            --accent: #ffd166;
            --accent2: #06d6a0;
            --shadow: 0 12px 30px rgba(0, 0, 0, 0.28);
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            font-family: Cambria, serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(255, 209, 102, 0.20), transparent 28%),
                radial-gradient(circle at bottom right, rgba(6, 214, 160, 0.18), transparent 30%),
                linear-gradient(135deg, var(--bg1), var(--bg2));
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
        }

        .wrapper {
            width: 100%;
            max-width: 820px;
            background: rgba(255, 255, 255, 0.06);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 28px;
            box-shadow: var(--shadow);
            padding: 40px;
        }

        h1 {
            margin: 0 0 12px;
            font-size: 42px;
            font-weight: 700;
            letter-spacing: 0.5px;
        }

        .subtitle {
            margin: 0 0 30px;
            font-size: 18px;
            opacity: 0.88;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 18px;
        }

        a.card,
        button.card-btn {
            appearance: none;
            border: none;
            text-decoration: none;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 120px;
            padding: 22px;
            border-radius: 22px;
            background: var(--card);
            color: var(--text);
            box-shadow: 0 10px 22px rgba(0, 0, 0, 0.22);
            border: 1px solid rgba(255, 255, 255, 0.14);
            transition: transform 0.2s ease, background 0.2s ease, box-shadow 0.2s ease;
            font-family: Cambria, serif;
            font-size: 22px;
            font-weight: 700;
            text-align: center;
        }

        a.card:hover,
        button.card-btn:hover {
            transform: translateY(-4px);
            background: var(--card-hover);
            box-shadow: 0 16px 30px rgba(0, 0, 0, 0.30);
        }

        a.card.employees {
            border-left: 6px solid var(--accent);
        }

        a.card.add {
            border-left: 6px solid var(--accent2);
        }

        button.card-btn.export {
            border-left: 6px solid #8ecae6;
            width: 100%;
        }

        .footer {
            margin-top: 24px;
            font-size: 14px;
            opacity: 0.75;
            text-align: center;
        }

        .export-form {
            margin: 0;
            width: 100%;
        }
    </style>
</head>
<body>
    <div class="wrapper">
        <h1>Управление сотрудниками</h1>
        <p class="subtitle">Выберите действие ниже — быстро и удобно.</p>

        <div class="grid">
            <a class="card employees" href="/employees">Просмотреть сотрудников</a>
            <a class="card add" href="/add">Добавить сотрудника</a>

            <form class="export-form" action="/export" method="get">
                <button class="card-btn export" type="submit">Выгрузить на кассы</button>
            </form>
        </div>

        <div class="footer">Frontol User Web Panel</div>
    </div>
</body>
</html>
)html";
}

std::string build_add_page() {
    return R"html(
<!doctype html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Добавить сотрудника</title>
    <style>
        :root {
            --bg1: #1f2a44;
            --bg2: #243b55;
            --card: rgba(255, 255, 255, 0.08);
            --card-border: rgba(255, 255, 255, 0.14);
            --text: #f5f5f5;
            --accent: #ffd166;
            --accent2: #06d6a0;
            --input-bg: rgba(255, 255, 255, 0.12);
            --input-focus: rgba(255, 255, 255, 0.18);
            --shadow: 0 12px 30px rgba(0, 0, 0, 0.28);
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            font-family: Cambria, serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(255, 209, 102, 0.20), transparent 28%),
                radial-gradient(circle at bottom right, rgba(6, 214, 160, 0.18), transparent 30%),
                linear-gradient(135deg, var(--bg1), var(--bg2));
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
        }

        .wrapper {
            width: 100%;
            max-width: 700px;
            background: rgba(255, 255, 255, 0.06);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 28px;
            box-shadow: var(--shadow);
            padding: 40px;
        }

        h1 {
            margin: 0 0 12px;
            font-size: 40px;
            font-weight: 700;
            letter-spacing: 0.4px;
        }

        .subtitle {
            margin: 0 0 28px;
            font-size: 18px;
            opacity: 0.88;
        }

        form {
            display: grid;
            gap: 18px;
        }

        .field {
            display: grid;
            gap: 8px;
        }

        label {
            font-size: 18px;
            font-weight: 700;
            letter-spacing: 0.2px;
        }

        input[type="text"],
        select {
            width: 100%;
            padding: 14px 16px;
            border-radius: 16px;
            border: 1px solid var(--card-border);
            background: var(--input-bg);
            color: var(--text);
            outline: none;
            font-family: Cambria, serif;
            font-size: 18px;
            transition: background 0.2s ease, transform 0.2s ease, border-color 0.2s ease;
        }

        input[type="text"]::placeholder {
            color: rgba(245, 245, 245, 0.65);
        }

        select {
            background: rgba(31, 42, 68, 0.88);
            color: #f5f5f5;
        }

        select option {
            background: #243b55;
            color: #f5f5f5;
        }

        input[type="text"]:focus,
        select:focus {
            background: var(--input-focus);
            border-color: rgba(255, 255, 255, 0.28);
        }

        .actions {
            display: flex;
            gap: 14px;
            flex-wrap: wrap;
            margin-top: 18px;
        }

        button,
        .back-link {
            appearance: none;
            border: none;
            text-decoration: none;
            cursor: pointer;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            min-height: 56px;
            padding: 0 22px;
            border-radius: 18px;
            font-family: Cambria, serif;
            font-size: 20px;
            font-weight: 700;
            transition: transform 0.2s ease, box-shadow 0.2s ease, background 0.2s ease;
        }

        button {
            background: linear-gradient(135deg, #ffd166, #ffb703);
            color: #1f2a44;
            box-shadow: 0 12px 22px rgba(0, 0, 0, 0.22);
        }

        button:hover {
            transform: translateY(-3px);
            box-shadow: 0 16px 28px rgba(0, 0, 0, 0.28);
        }

        .back-link {
            background: rgba(255, 255, 255, 0.10);
            color: var(--text);
            border: 1px solid rgba(255, 255, 255, 0.14);
        }

        .back-link:hover {
            transform: translateY(-3px);
            background: rgba(255, 255, 255, 0.16);
        }

        .hint {
            margin-top: 4px;
            font-size: 14px;
            opacity: 0.75;
        }

        .error {
            display: none;
            color: #ff8fa3;
            font-size: 14px;
            font-weight: 700;
            margin-top: 2px;
        }
    </style>
</head>
<body>
    <div class="wrapper">
        <h1>Добавить сотрудника</h1>
        <p class="subtitle">Заполните данные сотрудника и нажмите кнопку добавления.</p>

        <form id="addForm" action="/add" method="post" novalidate>
            <div class="field">
                <label for="name">Имя сотрудника</label>
                <input
                    type="text"
                    id="name"
                    name="name"
                    required
                    placeholder="Например: Иван Иванов">
                <div class="hint">Имя должно начинаться с буквы и содержать только буквы, пробелы или дефисы.</div>
                <div class="error" id="nameError">Имя введено некорректно.</div>
            </div>

            <div class="field">
                <label for="pin_card">Номер карты</label>
                <input
                    type="text"
                    id="pin_card"
                    name="pin_card"
                    placeholder="Если пусто — сгенерируется автоматически"
                    inputmode="numeric">
                <div class="hint">Можно оставить поле пустым. Если заполняете — только цифры.</div>
                <div class="error" id="pinError">Номер карты должен содержать только цифры.</div>
            </div>

            <div class="field">
                <label for="role">Права</label>
                <select id="role" name="role" required>
                    <option value="1">кассир</option>
                    <option value="2">старший кассир</option>
                </select>
            </div>

            <div class="actions">
                <button type="submit">Добавить</button>
                <a class="back-link" href="/">Назад</a>
            </div>
        </form>
    </div>

    <script>
        const form = document.getElementById('addForm');
        const nameInput = document.getElementById('name');
        const pinInput = document.getElementById('pin_card');
        const nameError = document.getElementById('nameError');
        const pinError = document.getElementById('pinError');

        function isValidName(value) {
            const re = /^[A-Za-zА-Яа-яЁё]+([A-Za-zА-Яа-яЁё -]*[A-Za-zА-Яа-яЁё])?$/u;
            return re.test(value.trim());
        }

        function isValidPin(value) {
            return value === "" || /^[0-9]+$/.test(value);
        }

        form.addEventListener('submit', function (e) {
            const nameValue = nameInput.value.trim();
            const pinValue = pinInput.value.trim();

            let ok = true;

            if (!isValidName(nameValue)) {
                nameError.style.display = 'block';
                ok = false;
            } else {
                nameError.style.display = 'none';
            }

            if (!isValidPin(pinValue)) {
                pinError.style.display = 'block';
                ok = false;
            } else {
                pinError.style.display = 'none';
            }

            if (!ok) {
                e.preventDefault();
            }
        });

        nameInput.addEventListener('input', function () {
            if (isValidName(nameInput.value)) {
                nameError.style.display = 'none';
            }
        });

        pinInput.addEventListener('input', function () {
            if (isValidPin(pinInput.value)) {
                pinError.style.display = 'none';
            }
        });
    </script>
</body>
</html>
)html";
}

std::string build_employees_page(const Users &users) {
    std::ostringstream out;
    out << R"html(
<!doctype html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Сотрудники</title>
    <style>
        :root {
            --bg1: #1f2a44;
            --bg2: #243b55;
            --card: rgba(255, 255, 255, 0.08);
            --card-border: rgba(255, 255, 255, 0.14);
            --text: #f5f5f5;
            --accent: #ffd166;
            --danger: #ef476f;
            --danger-hover: #ff5d84;
            --shadow: 0 12px 30px rgba(0, 0, 0, 0.28);
            --row: rgba(255, 255, 255, 0.06);
            --row-hover: rgba(255, 255, 255, 0.10);
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            font-family: Cambria, serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(255, 209, 102, 0.20), transparent 28%),
                radial-gradient(circle at bottom right, rgba(6, 214, 160, 0.18), transparent 30%),
                linear-gradient(135deg, var(--bg1), var(--bg2));
            padding: 24px;
        }

        .wrapper {
            width: 100%;
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.06);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 28px;
            box-shadow: var(--shadow);
            padding: 34px;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            gap: 20px;
            flex-wrap: wrap;
            margin-bottom: 24px;
        }

        h1 {
            margin: 0;
            font-size: 40px;
            font-weight: 700;
            letter-spacing: 0.4px;
        }

        .subtitle {
            margin: 8px 0 0;
            font-size: 18px;
            opacity: 0.88;
        }

        .top-actions {
            display: flex;
            gap: 12px;
            flex-wrap: wrap;
        }

        .nav-btn {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            min-height: 54px;
            padding: 0 20px;
            border-radius: 18px;
            text-decoration: none;
            font-family: Cambria, serif;
            font-size: 18px;
            font-weight: 700;
            color: var(--text);
            background: rgba(255, 255, 255, 0.10);
            border: 1px solid rgba(255, 255, 255, 0.14);
            transition: transform 0.2s ease, background 0.2s ease, box-shadow 0.2s ease;
        }

        .nav-btn:hover {
            transform: translateY(-3px);
            background: rgba(255, 255, 255, 0.16);
            box-shadow: 0 14px 24px rgba(0, 0, 0, 0.22);
        }

        .table-wrap {
            overflow-x: auto;
            border-radius: 22px;
            border: 1px solid rgba(255, 255, 255, 0.10);
        }

        table {
            width: 100%;
            border-collapse: collapse;
            min-width: 900px;
            background: rgba(255, 255, 255, 0.05);
        }

        thead th {
            background: rgba(255, 255, 255, 0.12);
            color: #fff;
            text-align: left;
            padding: 16px 18px;
            font-size: 18px;
            letter-spacing: 0.2px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.14);
        }

        tbody td {
            padding: 16px 18px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.08);
            font-size: 18px;
        }

        tbody tr {
            background: var(--row);
            transition: background 0.2s ease, transform 0.2s ease;
        }

        tbody tr:hover {
            background: var(--row-hover);
        }

        tbody tr:last-child td {
            border-bottom: none;
        }

        .role-badge {
            display: inline-flex;
            align-items: center;
            padding: 6px 12px;
            border-radius: 999px;
            font-size: 16px;
            font-weight: 700;
            background: rgba(255, 255, 255, 0.14);
            border: 1px solid rgba(255, 255, 255, 0.14);
        }

        .delete-form {
            margin: 0;
        }

        .delete-btn {
            appearance: none;
            border: none;
            cursor: pointer;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            min-height: 42px;
            padding: 0 16px;
            border-radius: 14px;
            font-family: Cambria, serif;
            font-size: 16px;
            font-weight: 700;
            color: #fff;
            background: linear-gradient(135deg, var(--danger), #c9184a);
            box-shadow: 0 10px 18px rgba(0, 0, 0, 0.20);
            transition: transform 0.2s ease, box-shadow 0.2s ease, background 0.2s ease;
        }

        .delete-btn:hover {
            transform: translateY(-2px);
            background: linear-gradient(135deg, var(--danger-hover), #e0315b);
            box-shadow: 0 14px 22px rgba(0, 0, 0, 0.26);
        }

        .empty {
            padding: 26px 18px;
            text-align: center;
            font-size: 20px;
            opacity: 0.9;
        }
    </style>
</head>
<body>
    <div class="wrapper">
        <div class="header">
            <div>
                <h1>Сотрудники</h1>
                <p class="subtitle">Список сотрудников, их роли, номера карт и пароли.</p>
            </div>
            <div class="top-actions">
                <a class="nav-btn" href="/">Назад</a>
                <a class="nav-btn" href="/add">Добавить сотрудника</a>
            </div>
        </div>

        <div class="table-wrap">
            <table>
                <thead>
                    <tr>
                        <th>Имя</th>
                        <th>Права</th>
                        <th>Номер карты</th>
                        <th>Пароль</th>
                        <th>Удалить</th>
                    </tr>
                </thead>
                <tbody>
)html";

    if (users.empty()) {
        out << R"html(
                    <tr>
                        <td class="empty" colspan="5">Список сотрудников пока пуст.</td>
                    </tr>
)html";
    } else {
        for (const auto &[name, info]: users) {
            out << "<tr>";
            out << "<td>" << html_escape(name) << "</td>";
            out << "<td><span class=\"role-badge\">" << html_escape(role_name(info.role)) << "</span></td>";
            out << "<td>" << info.pin_card << "</td>";
            out << "<td>" << info.password << "</td>";
            out << "<td>"
                << "<form class=\"delete-form\" action=\"/delete\" method=\"post\">"
                << "<input type=\"hidden\" name=\"name\" value=\"" << html_escape(name) << "\">"
                << "<button class=\"delete-btn\" type=\"submit\">Удалить</button>"
                << "</form>"
                << "</td>";
            out << "</tr>";
        }
    }

    out << R"html(
                </tbody>
            </table>
        </div>
    </div>
</body>
</html>
)html";

    return out.str();
}

bool write_kassa_file(const std::string &path, const std::string &f_name, const std::string &fl_name,
                      const Users &users) {
    std::ofstream flag_file(path + fl_name);
    if (!flag_file.is_open())
        return false;
    flag_file << "";
    flag_file.close();

    std::ostringstream text;
    text << "##@@&&\n#\n$$$DELETEALLUSERS\n$$$ADDUSERS\n";
    text << "1;Системный администратор;Системный администратор;1;147896325;;;\n";

    int count = 2;
    for (const auto &[name, info]: users) {
        if (info.role == 1) {
            text << count << ";" << name << ";" << name << ";4;" << info.password << ";;;\n";
        } else if (info.role == 2) {
            text << count << ";" << name << ";" << name << ";3;" << info.password << ";" << info.pin_card << ";;\n";
        }
        ++count;
    }

    std::ofstream main_file(path + f_name, std::ios::binary);
    if (!main_file.is_open())
        return false;
    main_file << text.str();
    return true;
}

void export_to_kassa() {
    Users users = load_info();
    write_kassa_file(PATH_KASSA1, "Pos01.spr", "Pos01.flz", users);
    write_kassa_file(PATH_KASSA2, "Pos02.spr", "Pos02.flz", users);
}

std::string build_export_page() {
    return R"html(
<!doctype html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Выгрузка выполнена</title>
    <style>
        :root {
            --bg1: #1f2a44;
            --bg2: #243b55;
            --text: #f5f5f5;
            --shadow: 0 12px 30px rgba(0, 0, 0, 0.28);
            --success1: #06d6a0;
            --success2: #2ec4b6;
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            font-family: Cambria, serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(255, 209, 102, 0.20), transparent 28%),
                radial-gradient(circle at bottom right, rgba(6, 214, 160, 0.18), transparent 30%),
                linear-gradient(135deg, var(--bg1), var(--bg2));
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
        }

        .wrapper {
            width: 100%;
            max-width: 680px;
            background: rgba(255, 255, 255, 0.06);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 28px;
            box-shadow: var(--shadow);
            padding: 42px;
            text-align: center;
        }

        .icon {
            width: 84px;
            height: 84px;
            margin: 0 auto 18px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 42px;
            background: linear-gradient(135deg, var(--success1), var(--success2));
            box-shadow: 0 14px 28px rgba(0, 0, 0, 0.25);
        }

        h1 {
            margin: 0 0 12px;
            font-size: 40px;
            font-weight: 700;
            letter-spacing: 0.4px;
        }

        p {
            margin: 0 0 28px;
            font-size: 20px;
            opacity: 0.9;
        }

        .actions {
            display: flex;
            justify-content: center;
            gap: 14px;
            flex-wrap: wrap;
        }

        a {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            min-height: 56px;
            padding: 0 24px;
            border-radius: 18px;
            text-decoration: none;
            font-family: Cambria, serif;
            font-size: 20px;
            font-weight: 700;
            color: #1f2a44;
            background: linear-gradient(135deg, #ffd166, #ffb703);
            box-shadow: 0 12px 22px rgba(0, 0, 0, 0.22);
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }

        a:hover {
            transform: translateY(-3px);
            box-shadow: 0 16px 28px rgba(0, 0, 0, 0.28);
        }
    </style>
</head>
<body>
    <div class="wrapper">
        <div class="icon">✓</div>
        <h1>Готово</h1>
        <p>Выгрузка сотрудников на кассы произведена успешно.</p>
        <div class="actions">
            <a href="/">Вернуться на главную</a>
        </div>
    </div>
</body>
</html>
)html";
}

int main() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(build_index_page(), "text/html; charset=utf-8");
    });

    svr.Get("/employees", [](const httplib::Request &, httplib::Response &res) {
        Users users = load_info();
        res.set_content(build_employees_page(users), "text/html; charset=utf-8");
    });

    svr.Get("/add", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(build_add_page(), "text/html; charset=utf-8");
    });

    svr.Post("/add", [](const httplib::Request &req, httplib::Response &res) {
        Users users = load_info();

        std::string name = req.get_param_value("name");
        std::string pin_card_str = req.get_param_value("pin_card");
        std::string role_str = req.get_param_value("role");

        if (name.empty()) {
            res.set_content("Имя не может быть пустым", "text/plain; charset=utf-8");
            return;
        }

        int role = 0;
        try {
            role = std::stoi(role_str);
        } catch (...) {
            res.set_content("Некорректная роль", "text/plain; charset=utf-8");
            return;
        }

        if (role != 1 && role != 2) {
            res.set_content("Роль должна быть 1 или 2", "text/plain; charset=utf-8");
            return;
        }

        int pin_card = 0;
        if (pin_card_str.empty()) {
            pin_card = random_int(1, 10000000);
        } else {
            try {
                pin_card = std::stoi(pin_card_str);
            } catch (...) {
                res.set_content("Некорректный номер карты", "text/plain; charset=utf-8");
                return;
            }
        }

        UserInfo info;
        info.password = random_int(1, 1000000);
        info.pin_card = pin_card;
        info.role = role;

        users[name] = info;
        if (!save_info(users)) {
            res.set_content("Не удалось сохранить файл", "text/plain; charset=utf-8");
            return;
        }

        res.set_redirect("/employees");
    });

    svr.Post("/delete", [](const httplib::Request &req, httplib::Response &res) {
        Users users = load_info();
        std::string name = req.get_param_value("name");

        auto it = users.find(name);
        if (it != users.end()) {
            users.erase(it);
            save_info(users);
        }

        res.set_redirect("/employees");
    });

    svr.Get("/export", [](const httplib::Request &, httplib::Response &res) {
        export_to_kassa();
        res.set_content(build_export_page(), "text/html; charset=utf-8");
    });

    std::cout << "Server started on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
