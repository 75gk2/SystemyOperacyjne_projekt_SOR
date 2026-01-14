#pragma once
class GenericIPC {
public:
    virtual ~GenericIPC() = default;
protected:
    explicit GenericIPC(bool isCreator);
    explicit GenericIPC() = default;
    [[nodiscard]] bool isThisCreator() const {
        return isCreator;
    }

    [[nodiscard]] int getFlag() const {
        return flag;
    }

private:

    bool isCreator{false};
    int flag{0600};
};
