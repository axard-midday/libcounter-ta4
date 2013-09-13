#include <libaskue/checksum.h>
#include <libaskue/macro.h>
#include <libaskue/uint8_array.h>
#include <libaskue/types.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <math.h>   // ЛИНКОВАТЬ С ОПЦИЕЙ -lm

#include "libcounter_ta4.h"

#define START                   0
#define ADDRESS                 1
#define PASSWORD                4
#define COMMAND                 9

static
size_t __cnt_ta4_init ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *Cmd )
{
    size_t Token = 0;
    // Стартовый символ
    Dest->Item[ START ] = '#';
    Token = 1;
    // Имя
    for ( size_t i = 0; i < 3; i++ ) Dest->Item[ i + ADDRESS ] = Addr[ i ];
    Token += 3;
    // Пароль
    for ( size_t i = 0; i < 5; i++ ) Dest->Item[ i + PASSWORD ] = Pwd[ i ];
    Token += 5;
    // команда
    for ( size_t i = 0; Cmd[ i ] != '\0'; i++ ) 
    {
        Dest->Item[ i + COMMAND ] = Cmd[ i ];
        Token ++;
    }
    return Token;
}

// перевод hex в char
static 
uint8_t __to_char ( uint8_t Nibble )
{
    if ( ( Nibble >= ( uint8_t ) 0 ) && ( Nibble <= ( uint8_t ) 9 ) )
    {
        return Nibble | ( uint8_t ) 0x30;
    }
    else if ( Nibble == ( uint8_t ) 0x0a )
    {
        return ( uint8_t ) 'A';
    }
    else if ( Nibble == ( uint8_t ) 0x0b )
    {
        return ( uint8_t ) 'B';
    }
    else if ( Nibble == ( uint8_t ) 0x0c )
    {
        return ( uint8_t ) 'C';
    }
    else if ( Nibble == ( uint8_t ) 0x0d )
    {
        return ( uint8_t ) 'D';
    }
    else if ( Nibble == ( uint8_t ) 0x0e )
    {
        return ( uint8_t ) 'E';
    }
    else if ( Nibble == ( uint8_t ) 0x0f )
    {
        return ( uint8_t ) 'F';
    }
    else 
    {
        return ( uint8_t ) 0xff;
    }
}

// перевод char в hex
static 
uint8_t __to_hex ( uint8_t Nibble )
{
    if ( ( Nibble >= ( uint8_t ) 0x30 ) && ( Nibble <= ( uint8_t ) 0x39 ) )
    {
        return Nibble & ( uint8_t ) ( ~(0x30) );
    }
    else if ( Nibble == ( uint8_t ) 'A' )
    {
        return ( uint8_t ) 0x0a;
    }
    else if ( Nibble == ( uint8_t ) 'B' )
    {
        return ( uint8_t ) 0x0b;
    }
    else if ( Nibble == ( uint8_t ) 'C' )
    {
        return ( uint8_t ) 0x0c;
    }
    else if ( Nibble == ( uint8_t ) 'D' )
    {
        return ( uint8_t ) 0x0d;
    }
    else if ( Nibble == ( uint8_t ) 'E' )
    {
        return ( uint8_t ) 0x0e;
    }
    else if ( Nibble == ( uint8_t ) 'F' )
    {
        return ( uint8_t ) 0x0f;
    }
    else 
    {
        return ( uint8_t ) 0xff;
    }
}

static
size_t __cnt_ta4_checksum ( uint8_array_t *Dest, size_t Token )
{
    // вычислить контрольную сумму
    uint8_t cs = checksum_simple ( Dest->Item, Token );
    // разделить на старший и младший ниббл
    uint8_t HNibble, LNibble;
    HNibble = ( cs & 0xf0 ) >> 4;
    LNibble = cs & 0x0f;
    // перевести в символы
    Dest->Item[ Token ] = __to_char ( HNibble );
    Token ++;
    Dest->Item[ Token ] = __to_char ( LNibble );
    Token ++;
    // результат
    return Token;
}

static
void __verify ( uint8_array_t *Dest )
{
    for ( size_t i = 0; i < Dest->Size; i++ )
    {
        if ( isalpha ( Dest->Item[ i ] ) )
            Dest->Item[ i ] = toupper ( Dest->Item[ i ] );
    }
}

// проверка адреса
static
int __is_valid_addr ( const char *addr )
{
    // проверка отличия от стандартной длины ( 3 символа )
    if ( strlen ( addr ) != 3 ) return FALSE;
    // проверка что все символы - цифры
    return isdigit ( addr[ 0 ] ) && isdigit ( addr[ 1 ] ) && isdigit ( addr[ 2 ] );
}

// проверка пароля
static
int __is_valid_pwd ( const char *pwd )
{
    int is_good_symb ( int x )
    {
        return isdigit ( x ) || ( isalpha ( x ) && isupper ( x ) );
    }
    
    // проверка отличия от стандартной длины ( 5 символов )
    if ( strlen ( pwd ) != 5 ) return FALSE;
    // проверка что все символы или цифры, 
    // или буквы в верхнем регистре
    return is_good_symb ( pwd[ 0 ] ) && is_good_symb ( pwd[ 1 ] ) &&
            is_good_symb ( pwd[ 2 ] ) && is_good_symb ( pwd[ 3 ] ) &&
            is_good_symb ( pwd[ 4 ] );
}

// проверка символа начального ответа
static
int __is_valid_begin_marker ( const uint8_array_t *Dest )
{
    return Dest->Item[ 0 ] == '~';
}

// проверка символа конечного ответа
static
int __is_valid_end_marker ( const uint8_array_t *Dest )
{
    return Dest->Item[ Dest->Size - 1 ] == '\r';
}

// проверка адреса в ответе 
static
int __is_equal_addr ( const uint8_array_t *Dest, const char *addr )
{
    return ( Dest->Item[ 1 ] == addr[ 0 ] ) &&
            ( Dest->Item[ 2 ] == addr[ 1 ] ) &&
            ( Dest->Item[ 3 ] == addr[ 2 ] );
}

// проверка функции
static
int __is_equal_cmd ( const uint8_array_t *Dest, const char *func )
{
    int R = 0;
    for ( size_t i = 4; ( i < Dest->Size ) && 
                        ( func[ i - 4 ] != '\0' ) && 
                        ( R = Dest->Item[ i ] == func[ i - 4 ] ); i++ );
    return R;
}

// проверка контрольной суммы
static
int __is_equal_checksum ( const uint8_array_t *Dest )
{
    // выделить контрольную сумму
    uint8_t HNibble = __to_hex ( Dest->Item[ Dest->Size - 3 ] ); 
    uint8_t LNibble = __to_hex ( Dest->Item[ Dest->Size - 2 ] );
    // собрать в один байт
    uint8_t cs = ( HNibble << 4 ) | LNibble;
    // проверить
    return cs == checksum_simple ( Dest->Item, Dest->Size - 3 );
}


/* 0 */
/* Простая команда, котороя не содержит изменяющихся данных */
static
int __cnt_ta4_simple_cmd ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *Cmd )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
             
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 12 + strlen ( Cmd ) );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, Cmd );
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* Проверка правильности ответа */
static
int __is_valid_answer ( const uint8_array_t *Dest, const char *Addr, const char *Cmd )
{
    if ( !__is_valid_addr ( Addr ) ) 
    { 
        return ASKUE_ERROR;
    }
    else if ( !__is_valid_begin_marker ( Dest ) ) 
    { 
        return E_CNT_TA4_BMARKER; 
    }
    else if ( !__is_valid_end_marker ( Dest ) ) 
    {
        return E_CNT_TA4_EMARKER; 
    }
    else if ( !__is_equal_addr ( Dest, Addr ) ) 
    { 
        return E_CNT_TA4_ADDR; 
    }
    else if ( !__is_equal_cmd ( Dest, Cmd ) ) 
    { 
        return E_CNT_TA4_CMD; 
    }
    else if ( !__is_equal_checksum ( Dest ) ) 
    { 
        return E_CNT_TA4_CS; 
    }
    else 
    { 
        return ASKUE_SUCCESS;
    }
}

/* Простой ответ */
static
int __cnt_ta4_simple_ans ( const uint8_array_t *Dest, const char *Addr, const char *Cmd, int *Result )
{
    int R = __is_valid_answer ( Dest, Addr, Cmd );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    ( *Result ) = ( Dest->Item[ 5 ] == 'Y' ) ? 1 :
                  ( Dest->Item[ 5 ] == 'N' ) ? 0 : -1;
    return ASKUE_SUCCESS;
}

/* 1 */
/* Установка адреса прибора */
int cnt_ta4_set_dev_addr ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *NewAddr )
{
    // проверка входных данных
    // имя
    if ( !__is_valid_addr ( Addr ) ) return ASKUE_ERROR;
    // пароль
    if ( !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // данные
    if ( !__is_valid_addr ( NewAddr ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 16 );
    
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "A" );
    // новый адрес
    for ( size_t i = 0; i < 3; i++ )
    {
        Dest->Item[ Token + i ] = NewAddr[ i ];
    }
    Token += 3;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _1 */
/* Разбор ответа на команду "Установка адреса прибора" */
int _cnt_ta4_set_dev_addr ( const uint8_array_t *Dest, const char *NewAddr, const char *OldAddr, int *Result )
{
    // проверка нового адреса
    int R = __is_valid_answer ( Dest, NewAddr, "A" );
    if ( R == E_CNT_TA4_ADDR )
    {
        // проверка старого адреса
        R = __is_valid_answer ( Dest, OldAddr, "A" );
    }
    // отслеживание других ошибок
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    ( *Result ) = ( Dest->Item[ 5 ] == 'Y' ) ? 1 : 0;
    return ASKUE_SUCCESS;
}

/* 2 */
/* Установка пароля прибора */
int cnt_ta4_set_dev_pwd ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *NewPwd )
{
    // проверка входных данных
    // имя
    if ( !__is_valid_addr ( Addr ) ) return ASKUE_ERROR;
    // пароль
    if ( !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // данные
    if ( !__is_valid_pwd ( NewPwd ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 18 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "P" );
    // новый пароль
    for ( size_t i = 0; i < 5; i++ )
    {
        Dest->Item[ Token + i ] = NewPwd[ i ];
    }
    Token += 5;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _2 */
/* Разбор ответа на команду "Установка пароля прибора" */
int _cnt_ta4_set_dev_pwd ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "P", Result );
}

// проверка вхождения в диапазон между двумя числами типа int
static 
int __is_betweeni ( int X, int Left, int Right )
{
    return ( X >= Left ) && ( X <= Right );
}

// проверка вхождения в диапазон между двумя числами типа double
static 
int __is_betweend ( double X, double Left, double Right )
{
    return ( X >= Left ) && ( X <= Right );
}

/* 3 */
/* Установка категории потребителя */
int cnt_ta4_set_ckat ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int CKat )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // данные
    if ( !__is_betweeni ( CKat, 0, 99 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 15 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "K" );
    // категория потребителя
    Dest->Item[ Token ] = ( CKat / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( CKat % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _3 */
/* Разбор ответа на команду "Установка категории потребителя" */
int _cnt_ta4_set_ckat ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "K", Result );
}

/* 4 */
/* Внутренний тест прибора */
int cnt_ta4_test ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "T" );
}

/* _4 */
/* Разбор ответа на команду "Внутренний тест прибора" */
int _cnt_ta4_test ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "T", Result );
}

/* 5 */
/* Установка порога энергии на месяц */
int cnt_ta4_set_elimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int CKat, int ELimit )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // Проверка категории потребителя
    if ( !__is_betweeni ( CKat, 0, 99 ) ) return ASKUE_ERROR;
    // Проверка лимита энергии
    if ( !__is_betweeni ( ELimit, 0, 9999 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 19 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "J" );
    // категория потребителя
    Dest->Item[ Token ] = ( CKat / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( CKat % 10 ) | 0x30;
    Token++;
    // лимит энергии
    Dest->Item[ Token ] = ( ELimit / 1000 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( ( ELimit / 100 ) % 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( ( ELimit / 10 ) % 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( ELimit % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* 5 */
/* Разбор ответа на команду "Установка порога энергии на месяц" */
int _cnt_ta4_set_elimit ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "J", Result );
}

/* 6 */
/* Запрос выполнения последней общей команды */
int cnt_ta4_do_last_com_cmd ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "Q" );
}

/* _6 */
/* Разбор ответа на команду "Запрос выполнения последней общей команды" */
int _cnt_ta4_do_last_com_cmd ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "Q", Result );
}

/* 7 */
/* Запросить текущий день недели, дату и время */
int cnt_ta4_get_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "D" );
}

static
int __cnt_ta4_get_time ( const uint8_array_t *Dest, const char *Addr, const char *Cmd, int *Result, struct tm *TM )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, Cmd, Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 ) // если прихол ожидаемый ответ
    {
        // день недели
        int wday = __to_hex ( Dest->Item[ 5 ] );
        // часы
        int hour = __to_hex ( Dest->Item[ 6 ] ) * 10 + __to_hex ( Dest->Item[ 7 ] );
        // минуты
        int minute = __to_hex ( Dest->Item[ 8 ] ) * 10 + __to_hex ( Dest->Item[ 9 ] );
        // секунды
        int second = __to_hex ( Dest->Item[ 10 ] ) * 10 + __to_hex ( Dest->Item[ 11 ] );
        // день месяца
        int day = __to_hex ( Dest->Item[ 12 ] ) * 10 + __to_hex ( Dest->Item[ 13 ] );
        // месяц
        int month = __to_hex ( Dest->Item[ 14 ] ) * 10 + __to_hex ( Dest->Item[ 15 ] );
        // год
        int year = __to_hex ( Dest->Item[ 16 ] ) * 10 + __to_hex ( Dest->Item[ 17 ] );
        
        // Проверка
        if ( !__is_betweeni( wday, 0, 6 ) || 
             !__is_betweeni( hour, 0, 23 ) ||
             !__is_betweeni( minute, 0, 59 ) ||
             !__is_betweeni( second, 0, 59 ) ||
             !__is_betweeni( day, 1, 31 ) ||
             !__is_betweeni( month, 1, 12 ) ||
             !__is_betweeni( year, 0, 99 ) )
        {
            return ASKUE_ERROR;
        }
        
        TM->tm_wday = wday;
        TM->tm_hour = hour;
        TM->tm_min = minute;
        TM->tm_sec = second;
        TM->tm_mday = day;
        TM->tm_mon = month - 1;
        TM->tm_year = 100 + year;
    }
    
    return ASKUE_SUCCESS;
}

/* _7 */
/* Разбор ответа на команду Запросить текущий день недели, дату и время" */
int _cnt_ta4_get_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM )
{
    return __cnt_ta4_get_time ( Dest, Addr, "D", Result, TM );
}

// прочитать начало тарифа
static
int __cnt_ta4_get_tariff_begin ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *Cmd, int WDay, int Mon )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // Проверка данных
    if ( !__is_betweeni ( WDay, 0, 7 ) ||
         !__is_betweeni ( Mon, 1, 12 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 16 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, Cmd );
    // день недели или праздник
    Dest->Item[ Token ] = WDay | 0x30;
    Token++;
    // месяц
    Dest->Item[ Token ] = ( Mon / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Mon % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

static 
int ___cnt_ta4_get_tariff_begin ( const uint8_array_t *Dest, const char *Addr, const char *Cmd, int *Result, int *Hour, int *Minute )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, Cmd, Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // часы
        int hour = __to_hex ( Dest->Item[ 5 ] ) * 10 + __to_hex ( Dest->Item[ 6 ] );
        // минуты
        int minute = __to_hex ( Dest->Item[ 7 ] ) * 10 + __to_hex ( Dest->Item[ 8 ] );
        
        // проверка
        if ( !__is_betweeni( hour, 0, 23 ) ||
             !__is_betweeni( minute, 0, 59 ) )
        {
            return ASKUE_ERROR;
        }
        
        ( *Hour ) = hour;
        ( *Minute ) = minute;
    }
    
    return ASKUE_SUCCESS;
}

/* 8 */
/* Прочитать начало установленного полупикового ( основного ) тарифа */
int cnt_ta4_get_main_begin ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon )
{
    return __cnt_ta4_get_tariff_begin ( Dest, Addr, Pwd, "B", WDay, Mon );
}

/* _8 */
/* Разбор ответа на команду "Прочитать начало установленного 
 * полупикового ( основного ) тарифа" */
int _cnt_ta4_get_main_begin ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Hour, int *Minute )
{
    return ___cnt_ta4_get_tariff_begin ( Dest, Addr, "B", Result, Hour, Minute );
}

/* 9 */
/* Прочитать начало установленного льготного тарифа */
int cnt_ta4_get_benefit_begin ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon )
{
    return __cnt_ta4_get_tariff_begin ( Dest, Addr, Pwd, "F", WDay, Mon );
}

/* _9 */
/* Разбор ответа на команду "Прочитать начало установленного льготного тарифа" */
int _cnt_ta4_get_benefit_begin ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Hour, int *Minute )
{
    return ___cnt_ta4_get_tariff_begin ( Dest, Addr, "F", Result, Hour, Minute );
}

/* 10 */
/* Чтение пикового расписания */
int cnt_ta4_get_peak_shedule ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon, int Tariff )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // Проверка данных
    if ( !__is_betweeni ( WDay, 0, 7 ) ||
         !__is_betweeni ( Mon, 1, 12 ) || 
         !__is_betweeni ( Tariff, 1, 3 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 16 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, ">" );
    // день недели или праздник
    Dest->Item[ Token ] = WDay | 0x30;
    Token++;
    // номер тарифа
    Dest->Item[ Token ] = Tariff | 0x30;
    Token++;
    // месяц
    Dest->Item[ Token ] = ( Mon / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Mon % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _10 */
/* Разбор ответа на команду "Чтение пикового расписания" */
int _cnt_ta4_get_peak_shedule ( const uint8_array_t *Dest, const char *Addr, int *Result, 
                                 int *Hour, int *Minute, int *Length, int *Tariff )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, ">", Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // часы
        int hour = __to_hex ( Dest->Item[ 5 ] ) * 10 + __to_hex ( Dest->Item[ 6 ] );
        // минуты
        int minute = __to_hex ( Dest->Item[ 7 ] ) * 10 + __to_hex ( Dest->Item[ 8 ] );
        // длина в минутах
        int length = __to_hex ( Dest->Item[ 9 ] ) * 100 + __to_hex ( Dest->Item[ 10 ] ) * 10 +
                     __to_hex ( Dest->Item[ 11 ] );
        // тариф
        int tariff = Dest->Item[ 12 ];
        
        // проверка
        if ( !__is_betweeni( hour, 0, 23 ) ||
             !__is_betweeni( minute, 0, 59 ) ||
             !__is_betweeni ( length, 0, 255 ) ||
             ( tariff != CNT_TA4_MAIN_TARIFF && 
               tariff != CNT_TA4_BENEFIT_TARIFF ) )
        {
            return ASKUE_ERROR;
        }
        
        ( *Hour ) = hour;
        ( *Minute ) = minute;
        ( *Length ) = length;
        ( *Tariff ) = tariff;
    }
    
    return ASKUE_SUCCESS;
}

/* 11 */
/* Прочитать установленную категорию потребителя */
int cnt_ta4_get_ckat ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "C" );
}

/* _11 */
/* Разбор ответа на команду "Прочитать установленную категорию потребителя." */
int _cnt_ta4_get_ckat ( const uint8_array_t *Dest, const char *Addr, int *Result, int *CKat )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "C", Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // категория потребителя
        int ckat = __to_hex ( Dest->Item[ 5 ] ) * 10 + __to_hex ( Dest->Item[ 6 ] );
        
        if ( !__is_betweeni ( ckat, 0, 99 ) ) return ASKUE_ERROR;
        
        ( *CKat ) = ckat;
    }
    
    return ASKUE_SUCCESS;
}

/* 12 */
/* Прочитать установленный лимит мощности */
int cnt_ta4_get_plimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "L" );
}

/* _12 */
/* Разбор ответа на команду "Прочитать установленный лимит мощности." */
int _cnt_ta4_get_plimit ( const uint8_array_t *Dest, const char *Addr, int *Result, double *PLimit )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "C", Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // категория потребителя
        int plimit = __to_hex ( Dest->Item[ 5 ] ) * 100 + 
                      __to_hex ( Dest->Item[ 6 ] ) * 10 +
                      __to_hex ( Dest->Item[ 6 ] );
        
        if ( !__is_betweeni ( plimit, 0, 511 ) ) return ASKUE_ERROR;
        
        ( *PLimit ) = plimit * 0.1;
    }
    
    return ASKUE_SUCCESS;
}

/* 13 */
/* Прочитать установленный лимит энергии */
int cnt_ta4_get_elimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "I" );
}

/* _13 */
/* Разбор ответа на команду "Прочитать установленный лимит энергии." */
int _cnt_ta4_get_elimit ( const uint8_array_t *Dest, const char *Addr, int *Result, int *ELimit )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "C", Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // категория потребителя
        int elimit = __to_hex ( Dest->Item[ 5 ] ) * 1000 + 
                      __to_hex ( Dest->Item[ 6 ] ) * 100 +
                      __to_hex ( Dest->Item[ 7 ] ) * 10 +
                      __to_hex ( Dest->Item[ 8 ] );
        
        if ( !__is_betweeni ( elimit, 0, 9999 ) ) return ASKUE_ERROR;
        
        ( *ELimit ) = elimit;
    }
    
    return ASKUE_SUCCESS;
}

/* 14 */ 
/* Прочитать суммарную потреблённую энергию по полупиковому ( основному ) тарифу. */
int cnt_ta4_get_main_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "E" );
}

static
int __cnt_ta4_get_esum ( const uint8_array_t *Dest, const char *Addr, const char *Cmd, int *Result, a_value_t *ESum )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, Cmd, Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        // категория потребителя
        double esum = ( double )( __to_hex ( Dest->Item[ 5 ] ) * 10000000 + 
                                    __to_hex ( Dest->Item[ 6 ] ) * 1000000 +
                                    __to_hex ( Dest->Item[ 7 ] ) * 100000 +
                                    __to_hex ( Dest->Item[ 8 ] ) * 10000 +
                                    __to_hex ( Dest->Item[ 9 ] ) * 1000 + 
                                    __to_hex ( Dest->Item[ 10 ] ) * 100 +
                                    __to_hex ( Dest->Item[ 11 ] ) * 10 +
                                    __to_hex ( Dest->Item[ 12 ] ) );
        // степень десяти
        int power = __to_hex ( Dest->Item[ 13 ] );
        
        if ( !__is_betweeni ( esum, 0, 99999999 ) ||
             !__is_betweeni ( esum, 0, 9 ) ) return ASKUE_ERROR;
        
        ESum->Base = esum;
        ESum->Exp = power;
    }
    
    return ASKUE_SUCCESS;
}

/* _14 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию
 * по полупиковому ( основному ) тарифу." */
int _cnt_ta4_get_main_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "E", Result, ESum );
}

/* 15 */ 
/* Прочитать суммарную потреблённую энергию по льготному тарифу. */
int cnt_ta4_get_benefit_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "W" );
}

/* _15 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по льготному тарифу." */
int _cnt_ta4_get_benefit_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "W", Result, ESum );
}

/* 16 */ 
/* Прочитать суммарную потреблённую энергию по полупиковому ( основному ) тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_main_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "X" );
}

/* _16 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по полупиковому ( основному ) тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_main_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "X", Result, ESum );
}

/* 17 */ 
/* Прочитать суммарную потреблённую энергию по льготному тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_benefit_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "Y" );
}

/* _17 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по льготному тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_benefit_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "Y", Result, ESum );
}

/* 18 */ 
/* Прочитать суммарную потреблённую энергию по пиковому тарифу. */
int cnt_ta4_get_peak_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "V" );
}

/* _18 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по пиковому тарифу." */
int _cnt_ta4_get_peak_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "V", Result, ESum );
}

/* 19 */ 
/* Прочитать суммарную потреблённую энергию по пиковому тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_peak_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "O" );
}

/* _19 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по пиковому тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_peak_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "V", Result, ESum );
}

/* 20 */ 
/* Прочитать суммарную потреблённую энергию по штрафному тарифу. */
int cnt_ta4_get_penalty_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "U" );
}

/* _20 */ 
/* Разбор ответа на команду "Прочитать суммарную 
 * потреблённую энергию по штрафному тарифу. */
int _cnt_ta4_get_penalty_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "U", Result, ESum );
}

/* 21 */
/* Запрос номера версии прибора */
int cnt_ta4_get_dev_version ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "R" );
}

/* _21 */
/* Разбор ответа на команду "Запрос номера версии прибора" */
int _cnt_ta4_get_dev_version ( const uint8_array_t *Dest, const char *Addr, int *Result,
                                char *Version, size_t len )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "R", Result );
    if ( R != ASKUE_SUCCESS )
    {
        return R;
    }
    
    if ( ( *Result ) == -1 )
    {
        if ( len <= 2 )
        {
            return ASKUE_ERROR;
        }
        
        Version[ 0 ] = Dest->Item[ 5 ];
        Version[ 1 ] = Dest->Item[ 6 ];
        Version[ 3 ] = '\0';
    }
    
    return ASKUE_SUCCESS;
}

/* 22 */
/* Запрос флага автоматического сезонного переключателя времени */
int cnt_ta4_get_season_flag ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "Z" );
}

/* _22 */
/* Разбор ответа на команду "Запрос флага автоматического
 * сезонного переключателя времени" */
int _cnt_ta4_get_season_flag ( const uint8_array_t *Dest, const char *Addr, int *Result,
                                int *Flag )
{
    // проверить на отрицательный ответ
    int R = __is_valid_answer ( Dest, Addr, "Z" );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    ( *Result ) = -1;
    ( *Flag ) = ( Dest->Item[ 5 ] == 'N' ) ? CNT_TA4_SEASON_FLAG_OFF :
                ( Dest->Item[ 5 ] == 'y' ) ? CNT_TA4_SEASON_FLAG_ON_MARCH :
                ( Dest->Item[ 5 ] == 'Y' ) ? CNT_TA4_SEASON_FLAG_ON_MARCH : 0; 
                
    if ( ( *Flag ) == 0 )
    {
        return ASKUE_ERROR;
    }
    else
    {
        return ASKUE_SUCCESS;
    }
}

/* 23 */
/* Чтение 12-символьного идентификатора счётчика */
int cnt_ta4_get_id ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "S" );
}

/* _23 */
/* Разбор ответа на команду "Чтение 12-символьного идентификатора счётчика" */
int _cnt_ta4_get_id ( const uint8_array_t *Dest, const char *Addr, int *Result,
                      char *Id, size_t Length )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "S", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    // на всякий случай, если символ получится
    if ( ( ( *Result ) != -1 && Dest->Size > ( size_t ) 9 ) ||
         ( *Result ) == -1 )
    {
        // проверка доступной строки
        if ( Length <= 12 ) return ASKUE_ERROR;
        
        // запись идентификатора
        for ( size_t i = 0; i < 12; i++ )
        {
            Id[ i ] = Dest->Item[ i + 5 ];
        }
        // завершить строку
        Id[ 12 ] = '\0';
    }
    
    return ASKUE_SUCCESS;
}

/* 24 */
/* Чтение даты и времени вскрытия крышки */
int cnt_ta4_get_open_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "\\T" );
}

/* _24 */
/* Разбор ответа на команду "Чтение даты и времени вскрытия крышки" */
int _cnt_ta4_get_open_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM )
{
    return __cnt_ta4_get_time ( Dest, Addr, "\\", Result, TM );
}

/* 25 */
/* Чтение даты и времени включения питания */
int cnt_ta4_get_on_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "\\P" );
}

/* _25 */
/* Разбор ответа на команду "Чтение даты и времени включения питания" */
int _cnt_ta4_get_on_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM )
{
    return __cnt_ta4_get_time ( Dest, Addr, "\\", Result, TM );
}

/* 26 */
/* Чтение даты и времени отключения питания */
int cnt_ta4_get_off_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "\\F" );
}

/* _26 */
/* Разбор ответа на команду "Чтение даты и времени отключения питания" */
int _cnt_ta4_get_off_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM )
{
    return __cnt_ta4_get_time ( Dest, Addr, "\\", Result, TM );
}

/* 27 */
/* Чтение режима индикации */
int cnt_ta4_get_display_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "^S" );
}

static
int __is_valid_display_mode ( const uint8_array_t *Dest )
{
    int R = 1;
    for ( size_t i = 0; ( i + 5 < Dest->Size ) && 
                        ( i < 8 ) &&
                        ( R = ( Dest->Item[ i + 5 ] == 'Y' ) || ( Dest->Item[ i + 5 ] == 'N' ) ); i++ );
    return R;
}
    
/* _27 */
/* Разбор ответа на команду "Чтение режима индикации" */
int _cnt_ta4_get_display_mode ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Mode, int *Period )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "Z", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( ( *Result ) != -1 && Dest->Size > ( size_t ) 9 ) ||
         ( *Result ) == -1 )
    {
        // период индикации
        int period = __to_hex ( Dest->Item[ 13 ] ) * 10 + __to_hex ( Dest->Item[ 14 ] );
        // проверка режима
        if ( !__is_valid_display_mode ( Dest ) ||
              !__is_betweeni ( period, 2, 60 ) ) return ASKUE_ERROR;
        // запись режима
        for ( size_t i = 0; ( i + 5 < Dest->Size ) && ( i < 8 ); i++ )
        {
            if ( Dest->Item[ i + 5 ] == 'Y' ) SETBIT ( ( *Mode ), i );
        }
        // запись периода
        ( *Period ) = period;
    }
    
    return ASKUE_SUCCESS;
}

/* 28 */
/* Установка режима индикации */
int cnt_ta4_set_display_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mode )
{
    uint8_t get_flag ( int mode, int place )
    {
        return ( uint8_t ) ( TESTBIT ( mode, place ) ) ? 'Y' : 'N';
    }
    
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 22 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "^W" );
    // установка флагов
    for ( int i = 7; i >= 0; i-- )
    {
        Dest->Item[ Token ] = get_flag ( Mode, i );
        Token++;
    }
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _28 */
/* Разбор ответа на команду "Установка режима индикации" */
int _cnt_ta4_set_display_mode ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "P", Result );
}

/* 29 */
/* Установка времени цикла индикации */
int cnt_ta4_set_display_period ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Time )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Time, 1, 60 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 16 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "^T" );
    // установка времени
    Dest->Item[ Token ] = ( Time / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Time % 10  ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _29 */
/* Разбор ответа на команду "Установка времени цикла индикации" */
int _cnt_ta4_set_display_period ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "^", Result );
}

// чтение показаний тарифа на начало суток 1-го числа месяца
static
int __cnt_ta4_get_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *Cmd, int Mon )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Mon, 1, 12 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 14 + strlen ( Cmd ) );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, Cmd );
    // установка времени
    Dest->Item[ Token ] = ( Mon / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Mon % 10  ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* 30 */
/* Чтение показаний льготного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_main_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon )
{
    return __cnt_ta4_get_e1mon ( Dest, Addr, Pwd, "[F", Mon );
}

/* 30 */
/* Разбор ответа на команду "Чтение показаний льготного тарифа
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_main_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "[", Result, E1Mon );
}

/* 31 */
/* Чтение показаний льготного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_benefit_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon )
{
    return __cnt_ta4_get_e1mon ( Dest, Addr, Pwd, "[B", Mon );
}

/* _31 */
/* Разбор ответа на команду "Чтение показаний льготного тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_benefit_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "[", Result, E1Mon );
}

/* 32 */
/* Чтение показаний пикового тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_peak_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon )
{
    return __cnt_ta4_get_e1mon ( Dest, Addr, Pwd, "[L", Mon );
}

/* _32 */
/* Разбор ответа на команду "Чтение показаний пикового тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_peak_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "[", Result, E1Mon );
}

/* 33 */
/* Чтение показаний штрафного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_penalty_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon )
{
    return __cnt_ta4_get_e1mon ( Dest, Addr, Pwd, "[P", Mon );
}

/* _33 */
/* Разбор ответа на команду "Чтение показаний штрафного тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_penalty_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon )
{
    return __cnt_ta4_get_esum ( Dest, Addr, "[", Result, E1Mon );
}

/* 34 */
/* Фиксация текущих показаний полупикового, льготного, пикового и 
 * штрафного на начало суток 1-го числа месяца */
int cnt_ta4_fix_all_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon )
{
    return __cnt_ta4_get_e1mon ( Dest, Addr, Pwd, "[P", Mon );
}

/* _34 */
/* Разбор ответа на команду "Фиксация текущих показаний полупикового, 
 * льготного, пикового и штрафного на начало суток 1-го числа месяца" */
int _cnt_ta4_fix_all_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "[", Result );
}

// правильная команда для выходного устройства
static
int __is_valid_output_mode ( int Mode )
{
    return ( Mode == 'T' ) || ( Mode == 'O' ) ||
            ( Mode == 'F' ) || ( Mode == 'C' );
}

/* 35 */
/* Установить режим функционирования выходного устройства */
int cnt_ta4_set_output_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mode )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_valid_output_mode ( Mode ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 14 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "G" );
    // установка времени
    Dest->Item[ Token ] = Mode;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _35 */
/* Разбор ответа на команду "Установить режим функционирования выходного устройства" */
int _cnt_ta4_set_output_mode ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "G", Result );
}

/* 36 */
/* Читать режим функционирования выходного устройства */
int cnt_ta4_get_output_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "GS" );
}

/* _36 */
/* Разбор ответа на команду "Читать режим функционирования выходного устройства" */
int _cnt_ta4_get_output_mode ( uint8_array_t *Dest, const char *Addr, int *Result, int *Mode )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "G", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // режим работы выходного устройства
        int mode = Dest->Item[ 5 ];
        // проверка
        if ( ( mode != CNT_TA4_OMODE_TELE ) ||
             ( mode != CNT_TA4_OMODE_ON ) ||
             ( mode != CNT_TA4_OMODE_OFF ) ||
             ( mode != CNT_TA4_OMODE_CTRL ) )
        {
            return ASKUE_ERROR;
        }
        
        ( *Mode ) = mode;
    }
    
    return ASKUE_SUCCESS;
}

// переключить что-либо
static
int __cnt_ta4_onoff ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *Cmd, int Flag )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 12 + strlen ( Cmd ) );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, Cmd );
    
    // проверка данных
    if ( ( Flag != CNT_TA4_ON ) && ( Flag != CNT_TA4_OFF ) ) return ASKUE_ERROR;
    
    // установка времени
    Dest->Item[ Token ] = Flag;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* 37 */
/* Разрешение/запрещение штрафного тарифа */
int cnt_ta4_set_penalty_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Flag )
{
    return __cnt_ta4_onoff ( Dest, Addr, Pwd, "_", Flag );
}

/* _37 */
/* Разбор ответа на команду "Разрешение/запрещение штрафного тарифа" */
int _cnt_ta4_set_penalty_state ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "_", Result );
}

/* 38 */
/* Читать статус разрешения штрафного тарифа */
int cnt_ta4_get_penalty_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "_S" );
}

/* _38 */
/* Разбор ответа на команду "Читать статус разрешения штрафного тарифа" */
int _cnt_ta4_get_penalty_state ( uint8_array_t *Dest, const char *Addr, int *Result, int *Flag )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "_", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // получение флага
        int flag = Dest->Item[ 5 ];
        // проверка
        if ( ( flag != CNT_TA4_ON ) && ( flag != CNT_TA4_OFF ) ) return ASKUE_ERROR;
        // возвращаемое значение
        ( *Flag ) = flag;
    }
    
    return ASKUE_SUCCESS;
}

/* 39 */
/* Чтение праздничного дня */
int cnt_ta4_get_holiday ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Number, 1, 16 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 16 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "]R" );
    // установка номера дня
    Dest->Item[ Token ] = ( Number / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Number % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _39 */
/* Разбор ответа на команду "Чтение праздничного дня" */
int _cnt_ta4_get_holiday ( uint8_array_t *Dest, const char *Addr, int *Result, int *Day, int *Month )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "]", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // день
        int day = __to_hex ( Dest->Item[ 5 ] ) * 10 + __to_hex ( Dest->Item[ 6 ] );
        // месяц
        int month = __to_hex ( Dest->Item[ 7 ] ) * 10 + __to_hex ( Dest->Item[ 8 ] );
        // проверка
        if ( !__is_betweeni ( day, 1, 31 ) ||
             !__is_betweeni ( month, 1, 12 ) ) return ASKUE_ERROR;
        
        // возвращаемое значение
        ( *Day ) = day;
        ( *Month ) = month;
    }
    
    return ASKUE_SUCCESS;
}

/* 40 */
/* Установка праздничного дня */
int cnt_ta4_set_holiday ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number, int Day, int Month )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Number, 1, 16 ) ||
         !__is_betweeni ( Day, 1, 31 ) || 
         !__is_betweeni ( Month, 1, 12 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 20 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "]W" );
    // установка номера дня
    Dest->Item[ Token ] = ( Number / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Number % 10 ) | 0x30;
    Token++;
    // установка дня
    Dest->Item[ Token ] = ( Day / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Day % 10 ) | 0x30;
    Token++;
    // установка месяца
    Dest->Item[ Token ] = ( Month / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Month % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _40 */
/* Разбор команды "Установка праздничного дня" */
int _cnt_ta4_set_holiday ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    // проверить на отрицательный ответ
    return __cnt_ta4_simple_ans ( Dest, Addr, "]", Result );
}

/* 41 */
/* Чтение даты последней оплаты полупикового ( основного ) тарифа */
int cnt_ta4_get_main_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "MB" );
}

// чтение даты последней оплаты
int __cnt_ta4_get_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Cmd, int *Result, 
                             int *Day, int *Month, int *Year, a_value_t *Value )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, Cmd, Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // день
        int day = __to_hex ( Dest->Item[ 5 ] ) * 10 + __to_hex ( Dest->Item[ 6 ] );
        // месяц
        int month = __to_hex ( Dest->Item[ 7 ] ) * 10 + __to_hex ( Dest->Item[ 8 ] );
        // год
        int year = __to_hex ( Dest->Item[ 9 ] ) * 10 + __to_hex ( Dest->Item[ 10 ] );
        // значение энергии
        double energy = ( double )( __to_hex ( Dest->Item[ 11 ] ) * 10000000 + 
                                    __to_hex ( Dest->Item[ 12 ] ) * 1000000 +
                                    __to_hex ( Dest->Item[ 13 ] ) * 100000 +
                                    __to_hex ( Dest->Item[ 14 ] ) * 10000 +
                                    __to_hex ( Dest->Item[ 15 ] ) * 1000 + 
                                    __to_hex ( Dest->Item[ 16 ] ) * 100 +
                                    __to_hex ( Dest->Item[ 17 ] ) * 10 +
                                    __to_hex ( Dest->Item[ 18 ] ) );
        // степень десяти
        int power = __to_hex ( Dest->Item[ 19 ] );
        // проверка
        if ( !__is_betweeni ( day, 1, 31 ) ||
             !__is_betweeni ( month, 1, 12 ) ||
             !__is_betweeni ( year, 0, 99 ) ||
             !__is_betweend ( energy, ( double ) 0, ( double ) 99999999 ) ||
             !__is_betweeni ( power, 0, 9 ) ) return ASKUE_ERROR;
        
        // возвращаемое значение
        ( *Day ) = day;
        ( *Month ) = month;
        ( *Year ) = year;
        Value->Base = energy;
        Value->Exp = power;
    }
    
    return ASKUE_SUCCESS;
}

/* _41 */
/* Разбор ответа на команду "Чтение даты последней оплаты полупикового ( основного ) тарифа" */
int _cnt_ta4_get_main_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                 int *Day, int *Month, int *Year, a_value_t *Value )
{
    return __cnt_ta4_get_last_pay ( Dest, Addr, "M", Result, Day, Month, Year, Value );
}

/* 42 */
/* Чтение даты последней оплаты льготного тарифа */
int cnt_ta4_get_benefit_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "MF" );
}


/* _42 */
/* Разбор ответа на команду "Чтение даты последней оплаты льготного тарифа" */
int _cnt_ta4_get_benefit_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                    int *Day, int *Month, int *Year, a_value_t *Value )
{
    return __cnt_ta4_get_last_pay ( Dest, Addr, "M", Result, Day, Month, Year, Value );
}

/* 43 */
/* Чтение даты последней оплаты пикового тарифа */
int cnt_ta4_get_peak_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "ML" );
}

/* _43 */
/* Разбор ответа на команду "Чтение даты последней оплаты пикового тарифа" */
int _cnt_ta4_get_peak_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                    int *Day, int *Month, int *Year, a_value_t *Value )
{
    return __cnt_ta4_get_last_pay ( Dest, Addr, "M", Result, Day, Month, Year, Value );
}

/* 44 */
/* Чтение обобщённого значения энергии на начало получаса 
 * для определения получасовой мощности */
int cnt_ta4_get_hhour_slice ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number, int Day, int Month )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Number, 0, 47 ) ||
         !__is_betweeni ( Day, 1, 31 ) || 
         !__is_betweeni ( Month, 1, 12 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 20 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "?" );
    // установка номера дня
    Dest->Item[ Token ] = ( Month / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Month % 10 ) | 0x30;
    Token++;
    // установка дня
    Dest->Item[ Token ] = ( Day / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Day % 10 ) | 0x30;
    Token++;
    // установка месяца
    Dest->Item[ Token ] = ( Number / 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Number % 10 ) | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _44 */
/* Разбор ответа на команду "Чтение обобщённого значения энергии 
 * на начало получаса для определения получасовой мощности" */
int _cnt_ta4_get_hhour_slice ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                int *Quarter, a_value_t *Value )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "?", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        if ( Dest->Item[ 5 ] == '?' ) return ASKUE_ERROR;
        
        // квартал
        int quarter = Dest->Item[ 5 ];
        // энергия
        double energy = ( double )( __to_hex ( Dest->Item[ 6 ] ) * 1000 + 
                                     __to_hex ( Dest->Item[ 7 ] ) * 100 +
                                     __to_hex ( Dest->Item[ 8 ] ) * 10 +
                                     __to_hex ( Dest->Item[ 9 ] ) );
        // степень 10
        int power = __to_hex ( Dest->Item[ 10 ] );
        // проверка
        if ( !__is_betweeni ( quarter, 0, 3 ) ||
             !__is_betweend ( energy, 0, ( double ) 99999999 ) ||
             !__is_betweeni ( power, 0, 9 ) )
        {
            return ASKUE_ERROR;
        }
        ( *Quarter ) = quarter;
        Value->Base = energy;
        Value->Exp = power;
    }
    
    return ASKUE_SUCCESS;
}

/* 45 */
/* Разрешение/запрещение однотарифного режима */
int cnt_ta4_set_onetariff_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Flag )
{
    return __cnt_ta4_onoff ( Dest, Addr, Pwd, "<", Flag );
}

/* _45 */
/* Разбор ответа от команды "Разрешение/запрещение однотарифного режима" */
int _cnt_ta4_set_onetariff_state ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "<", Result );
}

/* 46 */
/* Читать статус разрешения однотарифного режима */
int cnt_ta4_get_onetariff_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "<S" );
}

/* _46 */
/* Разбор ответа от команды "Читать статус разрешения однотарифного режима" */
int _cnt_ta4_get_onetariff_state ( uint8_array_t *Dest, const char *Addr, int *Result, int *Flag )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "<", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // получение флага
        int flag = Dest->Item[ 5 ];
        // проверка
        if ( ( flag != CNT_TA4_ON ) && ( flag != CNT_TA4_OFF ) ) return ASKUE_ERROR;
        // возвращаемое значение
        ( *Flag ) = flag;
    }
    
    return ASKUE_SUCCESS;
}

/* 47 */
/* Установить полный коэффициент трансформации */
int cnt_ta4_set_full_tk ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Tk, int Power )
{
    // проверка входных данных
    // имя и пароль
    if ( !__is_valid_addr ( Addr ) ||
         !__is_valid_pwd ( Pwd ) ) return ASKUE_ERROR;
    // проверка данных
    if ( !__is_betweeni ( Tk, 0, 999 ) || 
         !__is_betweeni ( Power, 0, 9 ) ) return ASKUE_ERROR;
    
    // подгон размера
    uint8_array_resize ( Dest, ( size_t ) 17 );
    // адрес, пароль и команда
    size_t Token = __cnt_ta4_init ( Dest, Addr, Pwd, "=W" );
    // установка коэффициента
    Dest->Item[ Token ] = ( Tk / 100 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( ( Tk / 10 ) % 10 ) | 0x30;
    Token++;
    Dest->Item[ Token ] = ( Tk / 10 ) | 0x30;
    Token++;
    // установка его степени
    Dest->Item[ Token ] = Power | 0x30;
    Token++;
    // контрольная сумма
    Token = __cnt_ta4_checksum ( Dest, Token );
    // Конечный символ
    Dest->Item[ Token ] = '\r';
    __verify( Dest );
    
    return ASKUE_SUCCESS;
}

/* _47 */
/* Разбор ответа от команды "Читать статус разрешения однотарифного режима" */
int _cnt_ta4_set_full_tk ( uint8_array_t *Dest, const char *Addr, int *Result )
{
    return __cnt_ta4_simple_ans ( Dest, Addr, "=", Result );
}

/* 48 */
/* Читать коэффициент трансформации */
int cnt_ta4_get_full_tk ( uint8_array_t *Dest, const char *Addr, const char *Pwd )
{
    return __cnt_ta4_simple_cmd ( Dest, Addr, Pwd, "=R" );
}

/* _48 */
/* Разбор ответа от команды "Читать коэффициент трансформации" */
int _cnt_ta4_get_full_tk ( uint8_array_t *Dest, const char *Addr, int *Result,
                           a_value_t *Tk )
{
    // проверить на отрицательный ответ
    int R = __cnt_ta4_simple_ans ( Dest, Addr, "=", Result );
    if ( R != ASKUE_SUCCESS ) 
    { 
        return R;
    }
    if ( ( *Result ) == -1 )
    {
        // коэффициент
        double tk = ( double )( __to_hex ( Dest->Item[ 5 ] ) * 100 + 
                                  __to_hex ( Dest->Item[ 6 ] ) * 10 +
                                  __to_hex ( Dest->Item[ 7 ] ) );
        // степень 10
        int power = __to_hex ( Dest->Item[ 10 ] );
        // проверка
        if ( !__is_betweeni ( power, 0, 9 ) ||
             !__is_betweend ( tk, ( double ) 0, ( double ) 999 ) )
        {
            return ASKUE_ERROR;
        }
        
        Tk->Base = tk;
        Tk->Exp = power;
    }
    
    return ASKUE_SUCCESS;
}

/* X */
/* Получить номер получасия */
int cnt_ta4_hhour_number ( const char *str, int *Number )
{
    // часы
    int hour = __to_hex ( str[ 0 ] ) * 10 + __to_hex ( str[ 1 ] );
    // минуты
    int minute = __to_hex ( str[ 3 ] ) * 10 + __to_hex ( str[ 4 ] );
    
    if ( !__is_betweeni ( hour, 0, 23 ) ||
         !__is_betweeni ( minute, 0, 59 ) ) return ASKUE_ERROR;
        
    ( *Number ) = hour * 2 + ( ( minute >= 30 ) ? 1: 0 );
    
    return ASKUE_SUCCESS;
    
}


#undef START
#undef ADDRESS
#undef PASSWORD
#undef COMMAND
