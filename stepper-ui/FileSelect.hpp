#ifndef FILESELECT_HPP
#define FILESELECT_HPP

#include <QWidget>
#include <QFileDialog>

#include <iostream>

typedef void (*buttonEvent)(QString);

class FileSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString prompt READ prompt CONSTANT)
    Q_PROPERTY(QString filter READ filter CONSTANT)
    Q_PROPERTY(buttonEvent processFileFunction READ processFileFunction CONSTANT)

public:
    FileSelect(
            QString m_prompt=tr("Select File"),
            QString m_defaultPath=QDir::currentPath(),
            QString m_filter=QString(),
            buttonEvent m_processFileFunction=nullptr)
            : QWidget(nullptr),
              m_prompt(m_prompt),
              m_filePath(QString()),
              m_filter(m_filter),
              m_defaultPath(m_defaultPath),
              m_processFileFunction(m_processFileFunction) {}

    Q_INVOKABLE
    void fileSelect() {
        QString filePath = QFileDialog::getOpenFileName(this, m_prompt, m_defaultPath, m_filter);

        if(!filePath.isEmpty()) {
            this->m_filePath = filePath;
            emit filePathChanged(filePath);

            QFileInfo qfi(filePath);
            m_defaultPath = qfi.absoluteFilePath();

            if(m_processFileFunction != nullptr) {
                m_processFileFunction(filePath);
            }
        }
    }

    QString filePath() {return m_filePath;}
    void setFilePath(QString filePath) {m_filePath = filePath;}

    QString prompt() {return m_prompt;}
    void setPrompt(QString prompt) {m_prompt = prompt;}

    QString filter() {return m_filter;}
    void setFilter(QString filter) {m_prompt = filter;}

    buttonEvent processFileFunction() {return m_processFileFunction;}

signals:
    void filePathChanged(QString path);

private:
    QString m_prompt;
    QString m_filePath;
    QString m_filter;
    QString m_defaultPath;

    buttonEvent m_processFileFunction;
};

#endif // FILESELECT_HPP
