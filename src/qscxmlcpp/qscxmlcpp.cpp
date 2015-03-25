/****************************************************************************
 **
 ** Copyright (c) 2015 Digia Plc
 ** For any questions to Digia, please use contact form at http://qt.digia.com/
 **
 ** All Rights Reserved.
 **
 ** NOTICE: All information contained herein is, and remains
 ** the property of Digia Plc and its suppliers,
 ** if any. The intellectual and technical concepts contained
 ** herein are proprietary to Digia Plc
 ** and its suppliers and may be covered by Finnish and Foreign Patents,
 ** patents in process, and are protected by trade secret or copyright law.
 ** Dissemination of this information or reproduction of this material
 ** is strictly forbidden unless prior written permission is obtained
 ** from Digia Plc.
 ****************************************************************************/

#include "../qscxmllib/scxmlparser.h"
#include "../qscxmllib/scxmlcppdumper.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args = a.arguments();
    QString usage = QStringLiteral("\nusage: %1 [-namespace <namespace>] [-o <base/out/name>] [-oh <header/out>] [-ocpp <cpp/out>] [-use-private-api]\n"
                                   "      [-basename <stateMachineClassName>] [-name-qobjects] <input.scxml>\n\n"
                                   "compiles the given input.scxml file to a header and cpp file\n")
            .arg(QFileInfo(args.value(0)).baseName());
    Scxml::CppDumpOptions options;
    QString scxmlFileName;
    QString outFileName;
    QString outHFileName;
    QString outCppFileName;
    for (int iarg = 1; iarg < args.size(); ++iarg) {
        QString arg = args.at(iarg);
        if (arg == QLatin1String("-namespace")) {
            options.namespaceName = args.value(++iarg);
        } else if (arg == QLatin1String("-o")) {
            outFileName = args.value(++iarg);
        } else if (arg == QLatin1String("-oh")) {
            outHFileName = args.value(++iarg);
        } else if (arg == QLatin1String("-ocpp")) {
            outCppFileName = args.value(++iarg);
        } else if (arg == QLatin1String("-use-private-api")) {
            options.usePrivateApi = true;
        } else if (arg == QLatin1String("-basename")) {
            options.basename = args.value(++iarg);
        } else if (arg == QLatin1String("-name-qobjects")) {
            options.nameQObjects = true;
        } else if (scxmlFileName.isEmpty()) {
            scxmlFileName = arg;
        } else {
            std::cerr << "Unexpected argument:" << arg.toStdString()
                      << usage.toStdString() << std::endl;
            exit(-1);
        }
    }
    if (scxmlFileName.isEmpty()) {
        std::cerr << "No input filename given:" << usage.toStdString() << std::endl;
        exit(-2);
    }
    QFile file(scxmlFileName);
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Error: could not open input file " << scxmlFileName.toStdString();
        exit(-3);
    }
    if (outFileName.isEmpty())
        outFileName = QFileInfo(scxmlFileName).baseName();
    if (outHFileName.isEmpty())
        outHFileName = outFileName + QLatin1String(".h");
    if (outCppFileName.isEmpty())
        outCppFileName = outFileName + QLatin1String(".cpp");

    QXmlStreamReader reader(&file);
    Scxml::ScxmlParser parser(&reader,
                              Scxml::ScxmlParser::loaderForDir(QFileInfo(file.fileName()).absolutePath()));
    parser.parse();

    QFile outH(outHFileName);
    if (!outH.open(QFile::WriteOnly)) {
        std::cerr << "Error: cannot open " << outH.fileName().toStdString()
                  << ": " << outH.errorString().toStdString() << usage.toStdString() << std::endl;
        exit(-4);
    }

    QFile outCpp(outCppFileName);
    if (!outCpp.open(QFile::WriteOnly)) {
        std::cerr << "Error: cannot open " << outCpp.fileName().toStdString()
                  << ": " << outCpp.errorString().toStdString()
                  << usage.toStdString() << std::endl;
        exit(-5);
    }

    QTextStream h(&outH);
    QTextStream c(&outCpp);
    Scxml::CppDumper dumper(h, c, outH.fileName(), options);
    dumper.dump(parser.table());
    outH.close();
    outCpp.close();

    a.exit();
    return 0;
}
