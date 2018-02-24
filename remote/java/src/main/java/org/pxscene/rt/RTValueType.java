package org.pxscene.rt;

/**
 * The all rtValue types.
 */
public enum RTValueType {
  VOID('\0'),
  VALUE('v'),
  BOOLEAN('b'),
  INT8('1'),
  UINT8('2'),
  INT32('4'),
  UINT32('5'),
  INT64('6'),
  UINT64('7'),
  FLOAT('e'),
  DOUBLE('d'),
  STRING('s'),
  OBJECT('o'),
  FUNCTION('f'),
  VOIDPTR('z'); // void ptr


  /**
   * the type code value
   */
  private char typeCode;


  /**
   * create new RTValueType with type code
   *
   * @param typeCode the type code value
   */
  RTValueType(char typeCode) {
    this.typeCode = typeCode;
  }

  /**
   * convert int value to RTValueType
   *
   * @param n the int value
   * @return the RTValueType entity
   */
  public static RTValueType fromTypeCode(int n) {
    for (RTValueType type : RTValueType.values()) {
      if (type.getTypeCode() == n) {
        return type;
      }
    }
    return RTValueType.VOID;
  }

  public char getTypeCode() {
    return this.typeCode;
  }
}
