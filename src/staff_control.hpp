#pragma once
#include <nlohmann/json.hpp>

#include "httplib.h"
struct Target {
    std::string name; // 32
    std::string path; // 32
    std::string file_name; // 32
    std::string flag_name; // 32

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Target, name, path)
};
struct Config {
    std::string path_db{}; // 32
    std::vector<Target> targets{}; // 24
    int port{}; // 4
    int log_level{}; // 4

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, path_db, targets, port, log_level)
};

struct UserInfo {
    int password{}; // 4
    int pin_card{}; // 4
    int role{}; // 4

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserInfo, password, pin_card, role)
};

using Users = std::map<std::string, UserInfo>;

class StaffControl {
public:
    StaffControl();
    void run();

private:
    [[nodiscard]] int random_int(int from, int to) const;
    [[nodiscard]] Users load_users() const;
    [[nodiscard]] std::string role_name(int role) const;
    [[nodiscard]] std::string html_escape(std::string_view s) const;
    [[nodiscard]] std::string build_error_page(std::string_view message) const;
    [[nodiscard]] std::string build_index_page() const;
    [[nodiscard]] std::string build_add_page() const;
    [[nodiscard]] std::string build_employees_page(const Users &users) const;
    [[nodiscard]] std::string build_export_page() const;
    void save_users(Users const &users);
    void write_kassa_file(std::string_view path, std::string_view f_name, std::string_view fl_name,
                          Users const &users) const;
    void export_to_kassa() const;

    httplib::Server m_server; // 752
    Config m_config; // 64
    std::string_view m_file_cfg{"/etc/staff-control/cfg.json"}; // 16
};
