#ifndef CONFIGKEYMAPPER_H_
#define CONFIGKEYMAPPER_H_

#include "WbKeyValues.h"
#include "WbConfig.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QSpinBox>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QGroupBox>

/** @brief Maps the value of a key in a config file to
 *  a widget on screen.
 *
 *  When the config is changed, the widget's display is updated.
 *  When the user edits the value the mapper emits a RequestCommit signal.
 */
class ConfigKeyMapper : public QObject
{
    Q_OBJECT
public:
    ConfigKeyMapper( const KeyName& keyToMap ):
        m_keyName( keyToMap )
        {
        };
    virtual ~ConfigKeyMapper(){};

    virtual void SetConfig( const WbConfig& config ) = 0;
    virtual void CommitData( WbConfig& config ) = 0;
    virtual bool Maps( QWidget* const widget ) const { Q_UNUSED(widget); return false; }

signals:
    void RequestCommit( ConfigKeyMapper& ) const;

protected slots:
    void DataChanged()
    {
        emit RequestCommit( *this );
    }

protected:
    KeyName m_keyName;
};

template< class WidgetClass >
class ConfigKeyMapperImplementation : public ConfigKeyMapper
{
public:
    ConfigKeyMapperImplementation( const KeyName& keyToMap, WidgetClass* widgetToMap )
    :
        ConfigKeyMapper( keyToMap ),
        m_widget( widgetToMap )
    {
        ConnectSignals();
    }
    virtual ~ConfigKeyMapperImplementation(){}

    inline virtual void SetConfig( const WbConfig& config );

    inline virtual void CommitData( WbConfig& config );

    virtual bool Maps( QWidget* const widget ) const
    {
        return ( widget == m_widget );
    }

private:
    inline void ConnectSignals();

    WidgetClass* m_widget;
};

//=================================================================================================================
// QSpinBox

template<>
inline
void ConfigKeyMapperImplementation< QSpinBox >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( editingFinished() ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QSpinBox >::SetConfig( const WbConfig& config )
{
    m_widget->setValue( config.GetKeyValue( m_keyName ).ToInt() );
}

template<>
inline
void ConfigKeyMapperImplementation< QSpinBox >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( QString().setNum( m_widget->value() ) ) );
}

//=================================================================================================================
// QDoubleSpinBox

template<>
inline
void ConfigKeyMapperImplementation< QDoubleSpinBox >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( editingFinished() ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QDoubleSpinBox >::SetConfig( const WbConfig& config )
{
    m_widget->setValue( config.GetKeyValue( m_keyName ).ToDouble() );
}

template<>
inline
void ConfigKeyMapperImplementation< QDoubleSpinBox >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( QString().setNum( m_widget->value() ) ) );
}

//=================================================================================================================
// QLineEdit

template<>
inline
void ConfigKeyMapperImplementation< QLineEdit >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( textChanged(const QString&) ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QLineEdit >::SetConfig( const WbConfig& config )
{
    //clear then insert to check validation before setting
    m_widget->clear();
    m_widget->insert( config.GetKeyValue( m_keyName ).ToQString() );
}

template<>
inline
void ConfigKeyMapperImplementation< QLineEdit >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( m_widget->text() ) );
}

//=================================================================================================================
// QPlainTextEdit

template<>
inline
void ConfigKeyMapperImplementation< QPlainTextEdit >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( textChanged() ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QPlainTextEdit >::SetConfig( const WbConfig& config )
{
    m_widget->setPlainText( config.GetKeyValue( m_keyName ).ToQString() );
}

template<>
inline
void ConfigKeyMapperImplementation< QPlainTextEdit >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( m_widget->toPlainText() ) );
}

//=================================================================================================================
// QComboBox

template<>
inline
void ConfigKeyMapperImplementation< QComboBox >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( currentIndexChanged(int) ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QComboBox >::SetConfig( const WbConfig& config )
{
    m_widget->setCurrentIndex(
        m_widget->findData( QVariant( config.GetKeyValue( m_keyName ).ToQString() ) ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QComboBox >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName,
                        KeyValue::from( m_widget
                                  ->itemData( m_widget->currentIndex() ).toString() ) );
}

//=================================================================================================================
// QCheckBox
template<>
inline
void ConfigKeyMapperImplementation< QCheckBox >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QCheckBox >::SetConfig( const WbConfig& config )
{
    m_widget->setChecked( config.GetKeyValue( m_keyName ).ToBool() );
}

template<>
inline
void ConfigKeyMapperImplementation< QCheckBox >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( m_widget->isChecked() ) );
}

//=================================================================================================================
// QGroupBox -- one that is checkable!
template<>
inline
void ConfigKeyMapperImplementation< QGroupBox >::ConnectSignals()
{
    QObject::connect( m_widget,
                      SIGNAL( toggled(bool) ),
                      this,
                      SLOT( DataChanged() ) );
}

template<>
inline
void ConfigKeyMapperImplementation< QGroupBox >::SetConfig( const WbConfig& config )
{
    m_widget->setChecked( config.GetKeyValue( m_keyName ).ToBool() );
}

template<>
inline
void ConfigKeyMapperImplementation< QGroupBox >::CommitData( WbConfig& config )
{
    config.SetKeyValue( m_keyName, KeyValue::from( m_widget->isChecked() ) );
}

#endif /* CONFIGKEYMAPPER_H_ */
