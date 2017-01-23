namespace cminor.db
{
    grammar CommandGrammar
    {
        Command: Command*;
        Spaces;
        FilePath: std::string;
    }
}
