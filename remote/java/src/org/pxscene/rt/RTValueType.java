package org.pxscene.rt;

public enum RTValueType {
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

  RTValueType(char typeCode) {
    m_typeCode = typeCode;
  }

  public char getTypeCode() {
    return m_typeCode;
  }

  public static RTValueType fromTypeCode(int n) {
    for (RTValueType type : RTValueType.values()) {
      if (type.getTypeCode() == n)
        return type;
    }
    return RTValueType.VOID;
  }

  private char m_typeCode;
}
