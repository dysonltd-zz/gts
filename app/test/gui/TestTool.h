#ifndef TESTTOOL_H
#define TESTTOOL_H

#include "Tool.h"
#include "MockQWidget.h"

class TestTool : public Tool
{
public:
    TestTool( WbSchema schema = WbSchema() )
        :
        Tool( 0, schema )
    {}

    virtual const QString Name() const { return QObject::tr( "Name" ); }

    const WbConfig TestGetCurrentConfig() const
    {
        return GetCurrentConfig();
    }

    static const WbSchema TestCreateWorkbenchSubSchema( const KeyName& schemaName, const QString& defaultName )
    {
        return CreateWorkbenchSubSchema( schemaName, defaultName );
    }

};

class TestToolWithDefaultSubSchemaFileName : public TestTool
{
public:
    TestToolWithDefaultSubSchemaFileName( const WbSchema& schema )
        :
        TestTool( schema )
    {}

    QString m_defaultSubSchemaFileName;

private:
    const QString GetSubSchemaDefaultFileName() const
    {
        return m_defaultSubSchemaFileName;
    }
};

class TestToolImplementingFullWorkbenchSchemaSubTree : public TestTool
{
public:
    TestToolImplementingFullWorkbenchSchemaSubTree( const WbSchema& schema )
        :
        TestTool( schema )
    {}
    WbSchema m_fullWorkBenchSchemaSubTree;

private:
    virtual const WbSchema GetFullWorkbenchSchemaSubTree() const
    {
        return m_fullWorkBenchSchemaSubTree;
    }
};

#endif
