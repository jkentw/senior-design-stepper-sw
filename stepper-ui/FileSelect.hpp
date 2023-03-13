#ifndef FILESELECT_HPP
#define FILESELECT_HPP

#include <QWidget>
#include <QFileDialog>

class FileSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString prompt READ prompt WRITE setPrompt CONSTANT)

public:
    FileSelect(
            QString m_prompt=tr("Select File"),
            QString defaultDirectory=QDir::homePath(),
            QString filter=QString())
            : QWidget(nullptr), m_prompt(m_prompt), defaultDirectory(defaultDirectory), filter(filter), m_filePath(QString()) {}

    Q_INVOKABLE
    void fileSelect() {
        QString filePath = QFileDialog::getOpenFileName(this, m_prompt, defaultDirectory, filter);

        if(!filePath.isEmpty()) {
            this->m_filePath = filePath;
            emit filePathChanged(filePath);
        }
    }

    QString filePath() {return m_filePath;}
    void setFilePath(QString filePath) {m_filePath = filePath;}

    QString prompt() {return m_prompt;}
    void setPrompt(QString prompt) {this->m_prompt=prompt;}

signals:
    void filePathChanged(QString path);

private:
    QString m_prompt;
    QString defaultDirectory;
    QString filter;
    QString m_filePath;
};

#endif // FILESELECT_HPP
