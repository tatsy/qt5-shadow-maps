#include "maingui.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qfiledialog.h>

#include "shadowmaps_widget.h"

// --
// MainGUI::InterfaceWidget class definition
// --

class MainGUI::InterfaceWidget : public QWidget {
public:
    // Public methods
    explicit InterfaceWidget(QWidget* parent = nullptr)
        : QWidget{ parent}
        , layout{ new QVBoxLayout }
        , saveButton{ new QPushButton }
        , smTypeGroup{ new QGroupBox }
        , groupLayout{ new QVBoxLayout }
        , smRadioButton{ new QRadioButton }
        , rsmRadioButton{ new QRadioButton } {        
        setLayout(layout);
        layout->setAlignment(Qt::AlignTop);

        // SM type radio buttons
        layout->addWidget(smTypeGroup);
        smTypeGroup->setLayout(groupLayout);
        smRadioButton->setText("SM");
        smRadioButton->setChecked(true);
        groupLayout->addWidget(smRadioButton);
        rsmRadioButton->setText("RSM");
        groupLayout->addWidget(rsmRadioButton);

        // Save button
        saveButton->setText("Save");
        layout->addWidget(saveButton);
    }

    ~InterfaceWidget() {
        delete groupLayout;
        delete smTypeGroup;
        delete saveButton;
        delete layout;
    }

    // Public fields
    QVBoxLayout* layout;
    QPushButton* saveButton;
    QGroupBox* smTypeGroup;
    QVBoxLayout* groupLayout;

    QRadioButton* smRadioButton;
    QRadioButton* rsmRadioButton;
};


// --
// MainGUI method definitions
// --

MainGUI::MainGUI(QWidget* parent) 
    : QMainWindow{ parent }
    , mainWidget{ new QWidget }
    , mainLayout{ new QGridLayout }
    , displayWidget{ nullptr }
    , ifaceWidget{ nullptr } {
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
    
    mainLayout->setColumnStretch(0, 4);
    mainLayout->setColumnStretch(1, 1);

    displayWidget = new ShadowMapsWidget(this);
    ifaceWidget = new InterfaceWidget(this);

    mainLayout->addWidget(displayWidget, 0, 0);
    mainLayout->addWidget(ifaceWidget, 0, 1);

    // Connect SIGNAL/SLOT
    connect(ifaceWidget->saveButton, SIGNAL(clicked()), this, SLOT(OnSaveButtonClicked()));
    connect(ifaceWidget->smRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonChanged(bool)));
    connect(ifaceWidget->rsmRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonChanged(bool)));
}

MainGUI::~MainGUI() {
    delete displayWidget;
    delete ifaceWidget;
    delete mainLayout;
    delete mainWidget;
}

void MainGUI::OnSaveButtonClicked() {
    QString savefile = 
        QFileDialog::getSaveFileName(this, tr("Save"), tr("image.jpg"), tr("JPEG (*.jpg);PNG (*.png)"));
    if (savefile == "") return;

    QImage image = displayWidget->grabFramebuffer();
    image.save(savefile);
}

void MainGUI::OnRadioButtonChanged(bool) {
    if (ifaceWidget->smRadioButton->isChecked()) {
        ((ShadowMapsWidget*)displayWidget)->setShadowMode(ShadowMaps::SM);
    } else if (ifaceWidget->rsmRadioButton->isChecked()) {
        ((ShadowMapsWidget*)displayWidget)->setShadowMode(ShadowMaps::RSM);
    }
    displayWidget->update();
}
