package org.pxscene.rt;

public enum SparkValueType {
  VOID('\0'),
  VALUE('v'),
  BOOLEAN('b'),
  INT8('1'),
  UINT('2'),
  INT32('4'),
  UINT32('5'),
  INT64('6'),
  UINT64('7'),
  FLOAT('e'),
  DOUBLE('d'),
  STRING('s'),
  OBJECT('o'),
  FUNCTION('f');

  SparkValueType(char typeCode) {
    m_typeCode = typeCode;
  }

  public char getTypeCode() {
    return m_typeCode;
  }

  public static SparkValueType fromTypeCode(int n) {
    for (SparkValueType type : SparkValueType.values()) {
      if (type.getTypeCode() == n)
        return type;
    }
    return SparkValueType.VOID;
  }

  private char m_typeCode;
}
