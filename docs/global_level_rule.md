
## 全局日志规则 

全局日志规则，应用有两个逻辑：
1. 在创建logger前，调用 `set_logger_level()` 设定指定的匹配规则。它将规则记录在注册表中备用。
2. 将一个logger注册到注册表时，它会查找是否有登记在册的日志等级设定，如果有，将它应用到当前logger中。

```c++
bool register_logger(std::shared_ptr<Logger> logger) 
{ 
    if (!logger) {
        return false;
    }

    __debug("register logger: %s", logger->name().c_str());

    // 应用全局日志等级规则（如果存在匹配的规则）
    auto level = detail::LoggerRegistry::instance().get_logger_level_rule(logger->name());
    if (level != LogLevel::Unknown) {
        logger->set_rule_level(level);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    registry_[logger->name()] = logger;
    return true;
}
```

3. 在logger创建后，调用`set_logger_level()` 设定指定的匹配规则，它会调用`apply_rules_to_existing_loggers()`, 将规则同步到所有已创建的logger中。

```c++
void apply_rules_to_existing_loggers(const std::string& pattern, LogLevel  level, bool is_regex) 
{
    if (is_regex) {
        // 正则表达式匹配：遍历所有 logger
        try {
            // 检查是否包含 shell 通配符，需要转换
            std::regex regex_pattern(regex_pattern_str, std::regex::ECMAScript | std::regex::optimize);
            for (auto& pair : registry_) {
                __debug("try match regex rule: %s to logger: %s", pattern.c_str(), pair.first.c_str());
                if (std::regex_match(pair.first, regex_pattern)) {
                    auto logger = pair.second;
                    logger->set_rule_level(level);
                    __debug("apply regex level rule to logger: %s", pair.first.c_str());
                }
            }
        } catch (const std::regex_error&) {
            // 正则表达式编译失败，忽略
        }
    } else {
        // 精确匹配
        for (auto& pair : registry_) {
            if (pair.first == pattern) {
                auto logger = pair.second;
                logger->set_rule_level(level);
                __debug("apply exact level rule to logger: %s", pair.first.c_str());
            }
        }
    }
}
```
