#include "maingui.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qfiledialog.h>

#include "common.h"
#include "openglviewer.h"

// --
// MainGUI::InterfaceWidget class definition
// --

class MainGUI::Ui : public QWidget {
public:
    // Public methods
    explicit Ui(QWidget* parent = nullptr)
        : QWidget{ parent}
        , layout{ new QVBoxLayout }
        , saveButton{ new QPushButton }
        , smTypeGroup{ new QGroupBox }
        , groupLayout{ new QVBoxLayout }
        , smRadioButton{ new QRadioButton }
        , rsmRadioButton{ new QRadioButton }
        , ismRadioButton{ new QRadioButton } {        
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
        ismRadioButton->setText("ISM");
        groupLayout->addWidget(ismRadioButton);

        // Save button
        saveButton->setText("Save");
        layout->addWidget(saveButton);
    }

    virtual ~Ui() {
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
    QRadioButton* ismRadioButton;
};


// --
// MainGUI method definitions
// --

MainGUI::MainGUI(QWidget* parent) 
    : QMainWindow{ parent }
    , mainWidget{ new QWidget }
    , mainLayout{ new QGridLayout }
    , viewer{ nullptr }
    , ui{ nullptr } {
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
    
    mainLayout->setColumnStretch(0, 4);
    mainLayout->setColumnStretch(1, 1);

    viewer = new OpenGLViewer(mainWidget);
    ui = new Ui(mainWidget);

    mainLayout->addWidget(viewer, 0, 0);
    mainLayout->addWidget(ui, 0, 1);

    // Connect SIGNAL/SLOT
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(OnSaveButtonClicked()));
    connect(ui->smRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonChanged(bool)));
    connect(ui->rsmRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonChanged(bool)));
    connect(ui->ismRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonChanged(bool)));
}

MainGUI::~MainGUI() {
    delete viewer;
    delete ui;
    delete mainLayout;
    delete mainWidget;
}

void MainGUI::OnSaveButtonClicked() {
    QString savefile = 
        QFileDialog::getSaveFileName(this, tr("Save"), 
                                     tr(OUTPUT_DIRECTORY) + tr("image.jpg"),
                                     tr("JPEG (*.jpg);PNG (*.png)"));
    if (savefile == "") return;

    QImage image = viewer->grabFramebuffer();
    image.save(savefile);
}

void MainGUI::OnRadioButtonChanged(bool) {
    if (ui->smRadioButton->isChecked()) {
        viewer->setShadowMode(ShadowMapType::SM);
    } else if (ui->rsmRadioButton->isChecked()) {
        viewer->setShadowMode(ShadowMapType::RSM);
    } else if (ui->ismRadioButton->isChecked()) {
        viewer->setShadowMode(ShadowMapType::ISM);
    }
}
