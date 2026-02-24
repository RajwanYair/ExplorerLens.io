#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace ExplorerLens::Tools::CLI {

    enum class CommandType {
        Generate,
        Batch,
        Info,
        Validate,
        Benchmark,
        Plugin,
        Help,
        Unknown
    };

    struct CommandContext {
        std::vector<std::wstring> Args;
        bool Verbose;
        bool JsonOutput;
    };

    class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual int Execute(const CommandContext& ctx) = 0;
        virtual std::wstring GetName() const = 0;
        virtual std::wstring GetDescription() const = 0;
        virtual std::wstring GetUsage() const = 0;
    };

    class CommandDispatcher {
    public:
        CommandDispatcher();
        
        void RegisterCommand(std::shared_ptr<ICommand> cmd);
        int Dispatch(int argc, wchar_t* argv[]);

    private:
        std::map<std::wstring, std::shared_ptr<ICommand>> m_commands;
        void PrintHelp();
    };

    // --- Command Stubs ---

    class GenerateCommand : public ICommand {
    public:
        int Execute(const CommandContext& ctx) override;
        std::wstring GetName() const override { return L"generate"; }
        std::wstring GetDescription() const override { return L"Generate a single thumbnail."; }
        std::wstring GetUsage() const override { return L"generate <input> <size> [output]"; }
    };

    class BatchCommand : public ICommand {
    public:
        int Execute(const CommandContext& ctx) override;
        std::wstring GetName() const override { return L"batch"; }
        std::wstring GetDescription() const override { return L"Process a list of files or folder."; }
        std::wstring GetUsage() const override { return L"batch <folder> <size> [-r/--recursive]"; }
    };

    class InfoCommand : public ICommand {
    public:
        int Execute(const CommandContext& ctx) override;
        std::wstring GetName() const override { return L"info"; }
        std::wstring GetDescription() const override { return L"Show information about a file or format."; }
        std::wstring GetUsage() const override { return L"info <file>"; }
    };
}

