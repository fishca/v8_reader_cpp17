#include <QtTest>
#include "v8reader/core/IV8Repository.h"

class V8CoreTest : public QObject {
    Q_OBJECT
private slots:
    void testRepositoryCreation() {
        auto repo = v8::core::createV8Repository();
        QVERIFY(repo != nullptr);
        QCOMPARE(repo->getRoot(), nullptr);
    }
};

QTEST_MAIN(V8CoreTest)
#include "V8File_test.moc"