#ifndef RISCV_SIM_SWITCHMAKER_H
#define RISCV_SIM_SWITCHMAKER_H

#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <utility>

// namespace Creator
// {
//     template<typename TBase, typename T, typename... Args>
//     static std::unique_ptr<TBase> CreateInstance(Args&&... args)
//     {
//         return std::unique_ptr<TBase>(new T(std::forward<Args>(args)...));
//     }
// }

template<typename SwitchType, typename Compare>
class SwitchMaker
{
    public:
        SwitchMaker() {}

        void AddCompare(Compare comp)
        {
            SwitchType type = comp->GetSwitchType();
            operation[type] = move(comp);
        }

        void DeleteCompare(SwitchType type)
        {
            operation.erase(type);
        }

        template<typename... Args>
        auto DoOperation(SwitchType type, Args&&... args)
        {
            if(operation.count(type) != 0)
                return (*operation[type])(std::forward<Args>(args)...);
            else if(defaultComp)
                return (**defaultComp)(std::forward<Args>(args)...);
            else
                throw std::invalid_argument("Unknown type");
        }

        void AddDefault(Compare comp)
        {
            defaultComp.emplace(move(comp));
        }

        Compare& GetCompare(SwitchType type) const
        {
            return operation.at(type);
        }

        Compare& GetDefaultCompare() const
        {
            return *defaultComp;
        }


    private:
        std::unordered_map<SwitchType, Compare> operation;
        std::optional<Compare> defaultComp;
};


#endif // RISCV_SIM_SWITCHMAKER_H