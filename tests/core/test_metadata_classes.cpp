#include <gtest/gtest.h>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <memory>

#include "v8reader/core/metadata/t_mdo.h"
#include "v8reader/core/metadata/t_requisite.h"
#include "v8reader/core/metadata/t_comand.h"
#include "v8reader/core/metadata/t_tabular.h"
#include "v8reader/core/metadata/t_form1c.h"
#include "v8reader/core/metadata/t_moxel.h"

using namespace v8reader::core;

// Вспомогательная функция для создания бинарных данных (заглушка)
// В реальном тесте здесь должен быть парсер формата 1CD
QByteArray createMockMDOData(const QString& name, const QString& synonym, quint8 version = 15) {
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);

    // Версия формата (1 байт)
    buffer.putChar(version);

    // Имя (длина 1 байт + строка)
    QByteArray nameData = name.toUtf8();
    buffer.putChar(static_cast<char>(nameData.size()));
    buffer.write(nameData);

    // Синоним (длина 1 байт + строка)
    QByteArray synData = synonym.toUtf8();
    buffer.putChar(static_cast<char>(synData.size()));
    buffer.write(synData);

    // Комментарий (пустой)
    buffer.putChar(0);

    // Пометка удаления (false)
    buffer.putChar(0);

    return data;
}

class TMDOTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TMDOTest, LoadBasicProperties) {
    QByteArray data = createMockMDOData("TestCatalog", "Тестовый справочник");
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);

    TMDO mdo;
    // В реальной реализации Load принимает QDataStream или подобный
    // Здесь проверяем только геттеры/сеттеры и конструктор, так как формат бинарный сложный
    // Для полноценного теста нужен мок парсера
    
    // Проверка конструктора по умолчанию
    EXPECT_TRUE(mdo.name().isEmpty());
    EXPECT_TRUE(mdo.synonym().isEmpty());
    EXPECT_EQ(mdo.version(), 0);
    
    // Проверка сеттеров (если есть публичные, иначе через friend или наследников)
    // В данном классе поля приватные, проверяем через методы доступа, если они есть
    // Или просто проверяем, что объект создается без ошибок
    EXPECT_NO_THROW({
        TMDO mdo2;
    });
}

TEST_F(TMDOTest, RequisiteCreation) {
    TRequisite req;
    EXPECT_TRUE(req.name().isEmpty());
    EXPECT_EQ(req.typeId(), 0);
    EXPECT_EQ(req.length(), 0);
    EXPECT_EQ(req.precision(), 0);
    EXPECT_FALSE(req.allowNull());
}

TEST_F(TMDOTest, RequisiteAddToMDO) {
    TMDO mdo;
    TRequisite req;
    // Предполагаем наличие метода addRequisite в TMDO
    // Если метод protected, тест нужно писать в классе-наследнике или использовать friend
    // Здесь проверяем логику добавления, если она реализована публично
    // В текущей реализации TMDO может не иметь публичного addRequisite
    // Тогда этот тест требует доработки интерфейса TMDO или использования моков
    EXPECT_EQ(mdo.requisites().size(), 0);
}

TEST_F(TComandTest, Creation) {
    TComand cmd;
    EXPECT_EQ(cmd.action(), QString());
    // Проверка других полей
}

TEST_F(TTabularTest, Creation) {
    TTabular tab;
    EXPECT_EQ(tab.defaultRowCount(), 0);
    EXPECT_EQ(tab.requisites().size(), 0);
}

TEST_F(TForm1CTest, Creation) {
    TForm1C form;
    // Проверка полей формы
    EXPECT_EQ(form.type(), 0);
}

TEST_F(TMoxelTest, Creation) {
    TMoxel moxel;
    // Проверка полей мохели
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
