#ifndef LIBCOUNTER_TA4_H_
#define LIBCOUNTER_TA4_H_

#include <libaskue/types.h>
#include <libaskue/uint8_array.h>

/*
 * Ошибки которые могут возникнуть при проверке.
 */
#define E_CNT_TA4_BMARKER       1
#define E_CNT_TA4_EMARKER       2
#define E_CNT_TA4_ADDR          3
#define E_CNT_TA4_CMD           4
#define E_CNT_TA4_CS            5

/*
 * Результат команды как простой ответ
 */
#define CNT_TA4_ANS_YES             1
#define CNT_TA4_ANS_NO              0
#define CNT_TA4_ANS_UNKNOWN         -1

/*
 * Виды тарифоф
 */
#define CNT_TA4_MAIN_TARIFF             'B'
#define CNT_TA4_BENEFIT_TARIFF          'L'

/*
 * Включение/выключение
 */
#define CNT_TA4_ON      'O'
#define CNT_TA4_OFF     'F'

/*
 * Варианты работы выходного устройства
 */
#define CNT_TA4_OMODE_TELE              'T'
#define CNT_TA4_OMODE_ON                'O'
#define CNT_TA4_OMODE_OFF               'F'
#define CNT_TA4_OMODE_CTRL              'C'

/*
 * Состояние флага зимнего времени
 */
#define CNT_TA4_SEASON_FLAG_ON_APRIL 'y'
#define CNT_TA4_SEASON_FLAG_ON_MARCH 'Y'
#define CNT_TA4_SEASON_FLAG_OFF 'N'

/* 1 */
/* Установка адреса прибора */
int cnt_ta4_set_dev_addr ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *NewAddr );

/* _1 */
/* Разбор ответа на команду "Установка адреса прибора" */
int _cnt_ta4_set_dev_addr ( const uint8_array_t *Dest, const char *NewAddr, const char *OldAddr, int *Result );

/* 2 */
/* Установка пароля прибора */
int cnt_ta4_set_dev_pwd ( uint8_array_t *Dest, const char *Addr, const char *Pwd, const char *NewPwd );

/* _2 */
/* Разбор ответа на команду "Установка пароля прибора" */
int _cnt_ta4_set_dev_pwd ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 3 */
/* Установка категории потребителя */
int cnt_ta4_set_ckat ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int CKat );

/* _3 */
/* Разбор ответа на команду "Установка категории потребителя" */
int _cnt_ta4_set_ckat ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 4 */
/* Внутренний тест прибора */
int cnt_ta4_test ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _4 */
/* Разбор ответа на команду "Внутренний тест прибора" */
int _cnt_ta4_test ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 5 */
/* Установка порога энергии на месяц */
int cnt_ta4_set_elimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int CKat, int ELimit );

/* 5 */
/* Разбор ответа на команду "Установка порога энергии на месяц" */
int _cnt_ta4_set_elimit ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 6 */
/* Запрос выполнения последней общей команды */
int cnt_ta4_do_last_com_cmd ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _6 */
/* Разбор ответа на команду "Запрос выполнения последней общей команды" */
int _cnt_ta4_do_last_com_cmd ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 7 */
/* Запросить текущий день недели, дату и время */
int cnt_ta4_get_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _7 */
/* Разбор ответа на команду Запросить текущий день недели, дату и время" */
int _cnt_ta4_get_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM );

/* 8 */
/* Прочитать начало установленного полупикового ( основного ) тарифа */
int cnt_ta4_get_main_begin ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon );

/* _8 */
/* Разбор ответа на команду "Прочитать начало установленного 
 * полупикового ( основного ) тарифа" */
int _cnt_ta4_get_main_begin ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Hour, int *Minute );

/* 9 */
/* Прочитать начало установленного льготного тарифа */
int cnt_ta4_get_benefit_begin ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon );

/* _9 */
/* Разбор ответа на команду "Прочитать начало установленного льготного тарифа" */
int _cnt_ta4_get_benefit_begin ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Hour, int *Minute );

/* 10 */
/* Чтение пикового расписания */
int cnt_ta4_get_peak_shedule ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int WDay, int Mon, int Tariff );

/* _10 */
/* Разбор ответа на команду "Чтение пикового расписания" */
int _cnt_ta4_get_peak_shedule ( const uint8_array_t *Dest, const char *Addr, int *Result, 
                                 int *Hour, int *Minute, int *Length, int *Tariff );
                                 
/* 11 */
/* Прочитать установленную категорию потребителя */
int cnt_ta4_get_ckat ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _11 */
/* Разбор ответа на команду "Прочитать установленную категорию потребителя." */
int _cnt_ta4_get_ckat ( const uint8_array_t *Dest, const char *Addr, int *Result, int *CKat );

/* 12 */
/* Прочитать установленный лимит мощности */
int cnt_ta4_get_plimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _12 */
/* Разбор ответа на команду "Прочитать установленный лимит мощности." */
int _cnt_ta4_get_plimit ( const uint8_array_t *Dest, const char *Addr, int *Result, double *PLimit );

/* 13 */
/* Прочитать установленный лимит энергии */
int cnt_ta4_get_elimit ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _13 */
/* Разбор ответа на команду "Прочитать установленный лимит энергии." */
int _cnt_ta4_get_elimit ( const uint8_array_t *Dest, const char *Addr, int *Result, int *ELimit );

/* 14 */ 
/* Прочитать суммарную потреблённую энергию по полупиковому ( основному ) тарифу. */
int cnt_ta4_get_main_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _14 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию
 * по полупиковому ( основному ) тарифу." */
int _cnt_ta4_get_main_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 15 */ 
/* Прочитать суммарную потреблённую энергию по льготному тарифу. */
int cnt_ta4_get_benefit_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _15 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по льготному тарифу." */
int _cnt_ta4_get_benefit_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 16 */ 
/* Прочитать суммарную потреблённую энергию по полупиковому ( основному ) тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_main_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _16 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по полупиковому ( основному ) тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_main_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 17 */ 
/* Прочитать суммарную потреблённую энергию по льготному тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_benefit_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _17 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по льготному тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_benefit_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 18 */ 
/* Прочитать суммарную потреблённую энергию по пиковому тарифу. */
int cnt_ta4_get_peak_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _18 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по пиковому тарифу." */
int _cnt_ta4_get_peak_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 19 */ 
/* Прочитать суммарную потреблённую энергию по пиковому тарифу 
 * с учётом превышения лимита мощности. */
int cnt_ta4_get_peak_esum_excess ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _19 */ 
/* Разбор ответа на команду "Прочитать суммарную потреблённую энергию 
 * по пиковому тарифу с учётом превышения лимита мощности." */
int _cnt_ta4_get_peak_esum_excess ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 20 */ 
/* Прочитать суммарную потреблённую энергию по штрафному тарифу. */
int cnt_ta4_get_penalty_esum ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _20 */ 
/* Разбор ответа на команду "Прочитать суммарную 
 * потреблённую энергию по штрафному тарифу. */
int _cnt_ta4_get_penalty_esum ( const uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *ESum );

/* 21 */
/* Запрос номера версии прибора */
int cnt_ta4_get_dev_version ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _21 */
/* Разбор ответа на команду "Запрос номера версии прибора" */
int _cnt_ta4_get_dev_version ( const uint8_array_t *Dest, const char *Addr, int *Result,
                                char *Version, size_t len );
                                
/* 22 */
/* Запрос флага автоматического сезонного переключателя времени */
int cnt_ta4_get_season_flag ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _22 */
/* Разбор ответа на команду "Запрос флага автоматического
 * сезонного переключателя времени" */
int _cnt_ta4_get_season_flag ( const uint8_array_t *Dest, const char *Addr, int *Result,
                                int *Flag );
                                
/* 23 */
/* Чтение 12-символьного идентификатора счётчика */
int cnt_ta4_get_id ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _23 */
/* Разбор ответа на команду "Чтение 12-символьного идентификатора счётчика" */
int _cnt_ta4_get_id ( const uint8_array_t *Dest, const char *Addr, int *Result,
                      char *Id, size_t Length );
                      
/* 24 */
/* Чтение даты и времени вскрытия крышки */
int cnt_ta4_get_open_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _24 */
/* Разбор ответа на команду "Чтение даты и времени вскрытия крышки" */
int _cnt_ta4_get_open_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM );

/* 25 */
/* Чтение даты и времени включения питания */
int cnt_ta4_get_on_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _25 */
/* Разбор ответа на команду "Чтение даты и времени включения питания" */
int _cnt_ta4_get_on_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM );

/* 26 */
/* Чтение даты и времени отключения питания */
int cnt_ta4_get_off_time ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _26 */
/* Разбор ответа на команду "Чтение даты и времени отключения питания" */
int _cnt_ta4_get_off_time ( const uint8_array_t *Dest, const char *Addr, int *Result, struct tm *TM );

/* 27 */
/* Чтение режима индикации */
int cnt_ta4_get_display_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _27 */
/* Разбор ответа на команду "Чтение режима индикации" */
int _cnt_ta4_get_display_mode ( const uint8_array_t *Dest, const char *Addr, int *Result, int *Mode, int *Period );

/* 28 */
/* Установка режима индикации */
int cnt_ta4_set_display_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mode );

/* _28 */
/* Разбор ответа на команду "Установка режима индикации" */
int _cnt_ta4_set_display_mode ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 29 */
/* Установка времени цикла индикации */
int cnt_ta4_set_display_period ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Time );

/* _29 */
/* Разбор ответа на команду "Установка времени цикла индикации" */
int _cnt_ta4_set_display_period ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 30 */
/* Чтение показаний льготного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_main_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon );

/* 30 */
/* Разбор ответа на команду "Чтение показаний льготного тарифа
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_main_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon );

/* 31 */
/* Чтение показаний льготного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_benefit_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon );

/* _31 */
/* Разбор ответа на команду "Чтение показаний льготного тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_benefit_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon );

/* 32 */
/* Чтение показаний пикового тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_peak_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon );

/* _32 */
/* Разбор ответа на команду "Чтение показаний пикового тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_peak_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon );

/* 33 */
/* Чтение показаний штрафного тарифа на начало суток 1-го числа месяца */
int cnt_ta4_get_penalty_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon );

/* _33 */
/* Разбор ответа на команду "Чтение показаний штрафного тарифа 
 * на начало суток 1-го числа месяца" */
int _cnt_ta4_get_penalty_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result, a_value_t *E1Mon );

/* 34 */
/* Фиксация текущих показаний полупикового, льготного, пикового и 
 * штрафного на начало суток 1-го числа месяца */
int cnt_ta4_fix_all_e1mon ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mon );

/* _34 */
/* Разбор ответа на команду "Фиксация текущих показаний полупикового, 
 * льготного, пикового и штрафного на начало суток 1-го числа месяца" */
int _cnt_ta4_fix_all_e1mon ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 35 */
/* Установить режим функционирования выходного устройства */
int cnt_ta4_set_output_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Mode );

/* _35 */
/* Разбор ответа на команду "Установить режим функционирования выходного устройства" */
int _cnt_ta4_set_output_mode ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 36 */
/* Читать режим функционирования выходного устройства */
int cnt_ta4_get_output_mode ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _36 */
/* Разбор ответа на команду "Читать режим функционирования выходного устройства" */
int _cnt_ta4_get_output_mode ( uint8_array_t *Dest, const char *Addr, int *Result, int *Mode );

/* 37 */
/* Разрешение/запрещение штрафного тарифа */
int cnt_ta4_set_penalty_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Flag );

/* _37 */
/* Разбор ответа на команду "Разрешение/запрещение штрафного тарифа" */
int _cnt_ta4_set_penalty_state ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 38 */
/* Читать статус разрешения штрафного тарифа */
int cnt_ta4_get_penalty_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _38 */
/* Разбор ответа на команду "Читать статус разрешения штрафного тарифа" */
int _cnt_ta4_get_penalty_state ( uint8_array_t *Dest, const char *Addr, int *Result, int *Flag );

/* 39 */
/* Чтение праздничного дня */
int cnt_ta4_get_holiday ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number );

/* _39 */
/* Разбор ответа на команду "Чтение праздничного дня" */
int _cnt_ta4_get_holiday ( uint8_array_t *Dest, const char *Addr, int *Result, int *Day, int *Month );

/* 40 */
/* Установка праздничного дня */
int cnt_ta4_set_holiday ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number, int Day, int Month );

/* _40 */
/* Разбор команды "Установка праздничного дня" */
int _cnt_ta4_set_holiday ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 41 */
/* Чтение даты последней оплаты полупикового ( основного ) тарифа */
int cnt_ta4_get_main_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _41 */
/* Разбор ответа на команду "Чтение даты последней оплаты полупикового ( основного ) тарифа" */
int _cnt_ta4_get_main_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                 int *Day, int *Month, int *Year, a_value_t *Value );
                                 
/* 42 */
/* Чтение даты последней оплаты льготного тарифа */
int cnt_ta4_get_benefit_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _42 */
/* Разбор ответа на команду "Чтение даты последней оплаты льготного тарифа" */
int _cnt_ta4_get_benefit_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                    int *Day, int *Month, int *Year, a_value_t *Value );
                                    
/* 43 */
/* Чтение даты последней оплаты пикового тарифа */
int cnt_ta4_get_peak_last_pay ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _43 */
/* Разбор ответа на команду "Чтение даты последней оплаты пикового тарифа" */
int _cnt_ta4_get_peak_last_pay ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                    int *Day, int *Month, int *Year, a_value_t *Value );

/* 44 */
/* Чтение обобщённого значения энергии на начало получаса 
 * для определения получасовой мощности */
int cnt_ta4_get_hhour_slice ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Number, int Day, int Month );

/* _44 */
/* Разбор ответа на команду "Чтение обобщённого значения энергии 
 * на начало получаса для определения получасовой мощности" */
int _cnt_ta4_get_hhour_slice ( uint8_array_t *Dest, const char *Addr, int *Result, 
                                int *Quarter, a_value_t *Value );
                                
/* 45 */
/* Разрешение/запрещение однотарифного режима */
int cnt_ta4_set_onetariff_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Flag );

/* _45 */
/* Разбор ответа от команды "Разрешение/запрещение однотарифного режима" */
int _cnt_ta4_set_onetariff_state ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 46 */
/* Читать статус разрешения однотарифного режима */
int cnt_ta4_get_onetariff_state ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _46 */
/* Разбор ответа от команды "Читать статус разрешения однотарифного режима" */
int _cnt_ta4_get_onetariff_state ( uint8_array_t *Dest, const char *Addr, int *Result, int *Flag );

/* 47 */
/* Установить полный коэффициент трансформации */
int cnt_ta4_set_full_tk ( uint8_array_t *Dest, const char *Addr, const char *Pwd, int Tk, int Power );

/* _47 */
/* Разбор ответа от команды "Читать статус разрешения однотарифного режима" */
int _cnt_ta4_set_full_tk ( uint8_array_t *Dest, const char *Addr, int *Result );

/* 48 */
/* Читать коэффициент трансформации */
int cnt_ta4_get_full_tk ( uint8_array_t *Dest, const char *Addr, const char *Pwd );

/* _48 */
/* Разбор ответа от команды "Читать коэффициент трансформации" */
int _cnt_ta4_get_full_tk ( uint8_array_t *Dest, const char *Addr, int *Result,
                           a_value_t *Tk );
                           
/* X */
/* Получить номер получасия */
int cnt_ta4_hhour_number ( const char *str, int *Number );

#endif /* LIBCOUNTER_TA4_H_ */
