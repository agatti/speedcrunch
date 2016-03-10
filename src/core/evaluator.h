// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2008, 2009, 2010, 2013 Helder Correia <helder.pereira.correia@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef CORE_EVALUATOR_H
#define CORE_EVALUATOR_H

#include "core/functions.h"
#include "math/hmath.h"
#include "math/cmath.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <core/opcode.h>
#include "core/variable.h"
#include "core/userfunction.h"

class Session;

class Token {
public:
    enum Op { InvalidOp = 0, Plus, Minus, Asterisk, Slash, Backslash, Caret,
              Super0, Super1, Super2, Super3, Super4, Super5, Super6, Super7, Super8, Super9,
              LeftPar, RightPar, Semicolon, Exclamation, Equal, Modulo,
              LeftShift, RightShift, Ampersand, Pipe, RightArrow };
    enum Type { stxUnknown, stxNumber, stxIdentifier, stxAbstract, stxOperator, stxOpenPar, stxClosePar, stxSep};
    //                     |<-------------isOperand------------->|<----------------isOperator----------------->|

    static const Token null;

    Token(Type type = stxUnknown, const QString& text = QString::null, int pos = -1);
    Token(const Token&);

    CNumber asNumber() const;
    Op asOperator() const;
    QString description() const;
    bool isNumber() const { return m_type == stxNumber; }
    bool isOperator() const { return m_type >= stxOperator; }
    bool isIdentifier() const { return m_type == stxIdentifier; }
    bool isOperand() const {return m_type == stxNumber || m_type == stxIdentifier || m_type == stxAbstract;}
    int pos() const { return m_pos; }
    QString text() const { return m_text; }
    void addUnit(QString t) { m_text = "->" + t;} // Needed for support of alphanumeric unit conversion operator "in"
    Type type() const { return m_type; }

    Token& operator=(const Token&);

protected:
    int m_pos;
    QString m_text;
    Type m_type;
};

class Tokens : public QVector<Token> {
public:
    Tokens() : QVector<Token>(), m_valid(true) { }

    bool valid() const { return m_valid; }
    void setValid(bool v) { m_valid = v; }

protected:
    bool m_valid;
};


class Evaluator : public QObject {
    Q_OBJECT

public:
    static Evaluator* instance();
    void reset();

    void setSession(Session * s);
    const Session *session();

    static bool isSeparatorChar(const QChar&);
    static bool isRadixChar(const QChar&);

    QString autoFix(const QString&);
    QString dump();
    QString error() const;
    CNumber eval();
    CNumber evalNoAssign();
    CNumber evalUpdateAns();
    QString expression() const;
    bool isValid();
    Tokens scan(const QString&) const;
    void setExpression(const QString&);
    Tokens tokens() const;
    bool isUserFunctionAssign() const;

    Variable getVariable(const QString&) const;
    QList<Variable> getVariables() const;
    QList<Variable> getUserDefinedVariables() const;
    QList<Variable> getUserDefinedVariablesPlusAns() const;
    void setVariable(const QString&, CNumber, Variable::Type = Variable::UserDefined);
    void unsetVariable(const QString&, bool force_builtin = false);
    void unsetAllUserDefinedVariables();
    bool isBuiltInVariable(const QString&) const;
    bool hasVariable(const QString&) const;
    void initializeBuiltInVariables();

    QList<UserFunction> getUserFunctions() const;
    void setUserFunction(const UserFunction & f);
    void unsetUserFunction(const QString&);
    void unsetAllUserFunctions();
    bool hasUserFunction(const QString&) const;

protected:
    void compile(const Tokens&);


private:
    Evaluator();
    Q_DISABLE_COPY(Evaluator)

    bool m_dirty;
    QString m_error;
    QString m_expression;
    bool m_valid;
    QString m_assignId;
    bool m_assignFunc;
    QStringList m_assignArg;
    QVector<Opcode> m_codes;
    QVector<CNumber> m_constants;
    QStringList m_identifiers;
    Session * m_session;
    QSet<QString> m_functionsInUse;

    const CNumber& checkOperatorResult(const CNumber&);
    static QString stringFromFunctionError(Function*);
    CNumber exec(const QVector<Opcode>& opcodes, const QVector<CNumber>& constants,
                 const QStringList& identifiers);
    CNumber execUserFunction(const UserFunction* function, QVector<CNumber>& arguments);
    const UserFunction * getUserFunction(const QString&) const;
};

#endif
