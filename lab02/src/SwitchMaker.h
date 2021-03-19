#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <utility>

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
        auto CrateInstance(SwitchType type, Args&&... args)
        {
            if(operation.count(type) != 0)
                return operation[type]->CreateInstance(std::forward<Args>(args)...);
            else if(defaultComp)
                return (*defaultComp)->CreateInstance(std::forward<Args>(args)...);
            else
                throw std::invalid_argument("Unknown type");
        }

        void AddDefault(Compare comp)
        {
            defaultComp.emplace(move(comp));
        }

        Compare& GetCompare(SwitchType type)
        {
            return operation.at(type);
        }

        Compare& GetDefaultCompare()
        {
            return *defaultComp;
        }


    private:
        std::unordered_map<SwitchType, Compare> operation;
        std::optional<Compare> defaultComp;
};