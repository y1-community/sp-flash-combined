#include "OkDialog.h"
#include "ui_OkDialog.h"
#include "../../Host/Inc/RuntimeMemory.h"
#include "../../Utility/IniItem.h"
#include "../../Utility/Theme.h"
#include <QShowEvent>

OkDialog::OkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OkDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

static const char *DONATE_INI_FILE = "option.ini";
static const char *DONATE_SECTION = "Donate";
static const char *DONATE_HIDE_KEY = "HideCoffeePrompt";

static IniItem donateSetting()
{
    return IniItem(DONATE_INI_FILE, DONATE_SECTION, DONATE_HIDE_KEY);
}

void OkDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // The dialog instance is reused, so refresh the prompt on every show.
    const bool suppressed = donateSetting().GetBooleanValue();
    ui->label_donate->setVisible(!suppressed);
    ui->checkBox_dontAsk->setVisible(!suppressed);
    if(!suppressed)
    {
        ui->checkBox_dontAsk->setChecked(false);
    }
    if(Theme::IsDarkActive())
    {
        // Keep the ok.png artwork but drop the blinding cyan fill.
        setStyleSheet("background-image:url(:/images/ok.png);"
                      "background-repeat:no-repeat;"
                      "background-position:center top;");
    }
}

void OkDialog::on_checkBox_dontAsk_toggled(bool checked)
{
    donateSetting().SaveBooleanValue(checked);
}

OkDialog::~OkDialog()
{
    delete ui;
}
