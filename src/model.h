#pragma once

#include <vector>
#include <string>
#include <map>

#include <stdexcept>
#include <functional>

namespace lp_pp
{

    class Variable
    {
    public:
        enum Type
        {
            INTEGER,
            CONTINUOUS,
        };

        Variable() = delete;
        Variable(std::string name, Type type, double objCoef = 0.0)
            : objCoef(objCoef), name(name), type(type){};

        double getObjValue() const
        {
            return objCoef;
        }

        void setObjCoef(double coef)
        {
            objCoef = coef;
        }

        std::string getName() const
        {
            return name;
        }

        void setName(std::string name)
        {
            this->name = name;
        }

        Type getType() const
        {
            return type;
        }

        void setType(Type newType)
        {
            type = newType;
        }

    private:
        double objCoef;
        std::string name;
        Type type;
    };

    bool operator<(const Variable &v1, const Variable &v2)
    {
        return v1.getName() < v2.getName();
    }

    using VarList = std::map<std::reference_wrapper<const Variable>, double>;

    class Constraint
    {
    public:
        enum BoundType
        {
            UP,
            DOWN,
            FREE,
            FIXED
        };

        Constraint() = delete;
        Constraint(std::string name, BoundType type, double value)
            : name(name), type(type), value(value){};

        std::string getName() const
        {
            return name;
        }

        void setName(std::string newName)
        {
            name = newName;
        }

        BoundType getBoundType()
        {
            return type;
        }

        double getBoundValue()
        {
            return value;
        }

        void setBound(BoundType type, double value)
        {
            this->type = type;
            this->value = value;
        }

        void setCoef(const Variable &var, double coef)
        {
            coefs[var] = coef;
        }

        double getCoef(const Variable &var)
        {
            return coefs.at(var);
        }

        const VarList &getCoefs()
        {
            return coefs;
        }

        void setCoefs(const VarList &coefs)
        {
            for (auto const &it : coefs)
            {
                this->coefs[it.first] = it.second;
            }
        }

    private:
        std::string name;
        std::map<std::reference_wrapper<const Variable>, double> coefs;
        BoundType type;
        double value;
    };

    using Matrix = std::vector<std::vector<double>>;

    class Model
    {
    public:
        Model() = delete;
        Model(const std::vector<Variable> &variables,
              const std::vector<Constraint> &constraints)
            : variables(variables), constraints(constraints){};

        size_t getNumCons()
        {
            return constraints.size();
        }

        size_t getNumVars()
        {
            return variables.size();
        }

        const std::vector<Variable> &getVariables()
        {
            return variables;
        }

        const std::vector<Constraint> &getConstraints()
        {
            return constraints;
        }

    private:
        std::vector<Variable> variables;
        std::vector<Constraint> constraints;
    };
}