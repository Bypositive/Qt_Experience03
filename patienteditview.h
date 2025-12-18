#ifndef PATIENTEDITVIEW_H
#define PATIENTEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class PatientEditView;
}

class PatientEditView : public QWidget
{
    Q_OBJECT

public:
    explicit PatientEditView(QWidget *parent = nullptr, bool isEditMode = false, const QString &patientId = "");
    ~PatientEditView();

    void setPatientData(const QString &patientId);

signals:
    void editFinished();

private slots:
    void on_save_clicked();
    void on_cancel_clicked();

private:
    Ui::PatientEditView *ui;
    bool editMode;
    QString currentPatientId;
    QDataWidgetMapper *mapper;
    void initUI();
    void loadPatientData();
    void savePatientData();
    bool validateInput();
};

#endif // PATIENTEDITVIEW_H
