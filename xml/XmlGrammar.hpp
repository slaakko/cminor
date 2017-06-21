#ifndef XmlGrammar_hpp_14187
#define XmlGrammar_hpp_14187

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/xml/XmlProcessor.hpp>

namespace cminor { namespace xml {

class XmlGrammar : public cminor::parsing::Grammar
{
public:
    static XmlGrammar* Create();
    static XmlGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, XmlProcessor* processor);
private:
    XmlGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class DocumentRule;
    class NameRule;
    class EntityValueRule;
    class AttValueRule;
    class SystemLiteralRule;
    class PubidLiteralRule;
    class CharDataRule;
    class CommentRule;
    class PIRule;
    class PITargetRule;
    class CDSectRule;
    class CDataRule;
    class PrologRule;
    class XMLDeclRule;
    class VersionInfoRule;
    class MiscRule;
    class DocTypeDeclRule;
    class DeclSepRule;
    class IntSubsetRule;
    class MarkupDeclRule;
    class ExtSubsetRule;
    class ExtSubsetDeclRule;
    class SDDeclRule;
    class ElementRule;
    class AttributeRule;
    class ETagRule;
    class ContentRule;
    class ElementDeclRule;
    class AttlistDeclRule;
    class AttDefRule;
    class DefaultDeclRule;
    class ConditionalSectRule;
    class IncludeSectRule;
    class CharRefRule;
    class ReferenceRule;
    class EntityRefRule;
    class PEReferenceRule;
    class EntityDeclRule;
    class GEDeclRule;
    class PEDeclRule;
    class EntityDefRule;
    class PEDefRule;
    class TextDeclRule;
    class ExtParsedEntRule;
    class EncodingDeclRule;
    class EncNameRule;
    class NotationDeclRule;
};

} } // namespace cminor.xml

#endif // XmlGrammar_hpp_14187
