/*!
 * \file WtEngine.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#pragma once
#include <queue>
#include <functional>
#include <stdint.h>
#include <unordered_set>
#include <unordered_map>

#include "RiskMonDefs.h"

#include "../Share/WTSMarcos.h"
#include "../Share/BoostDefine.h"
#include "../Share/DLLHelper.hpp"


NS_OTP_BEGIN
class WTSSessionInfo;
class WTSCommodityInfo;
class WTSContractInfo;

class IBaseDataMgr;
class IHotMgr;

class WTSVariant;

class WTSTickData;
struct WTSBarStruct;
class WTSKlineData;
class WTSHisTickData;
class WTSTickSlice;
class WTSKlineSlice;
class WTSPortFundInfo;

class WtDataManager;

typedef std::function<void()>	TaskItem;

class WtRiskMonWrapper
{
public:
	WtRiskMonWrapper(WtRiskMonitor* mon, IRiskMonitorFact* fact) :_mon(mon), _fact(fact){}
	~WtRiskMonWrapper()
	{
		if (_mon)
		{
			_fact->deleteRiskMonotor(_mon);
		}
	}

	WtRiskMonitor* self(){ return _mon; }


private:
	WtRiskMonitor*		_mon;
	IRiskMonitorFact*	_fact;
};
typedef std::shared_ptr<WtRiskMonWrapper>	WtRiskMonPtr;

class WtEngine : public WtPortContext
{
public:
	WtEngine();

	void set_date_time(uint32_t curDate, uint32_t curTime, uint32_t curSecs = 0, uint32_t rawTime = 0);

	void set_trading_date(uint32_t curTDate);

	inline uint32_t get_date() { return _cur_date; }
	inline uint32_t get_min_time() { return _cur_time; }
	inline uint32_t get_raw_time() { return _cur_raw_time; }
	inline uint32_t get_secs() { return _cur_secs; }
	inline uint32_t get_trading_date() { return _cur_tdate; }

	inline IBaseDataMgr*		get_basedata_mgr(){ return _base_data_mgr; }
	inline IHotMgr*				get_hot_mgr() { return _hot_mgr; }
	WTSSessionInfo*		get_session_info(const char* sid, bool isCode = false);
	WTSCommodityInfo*	get_commodity_info(const char* stdCode);
	WTSContractInfo*	get_contract_info(const char* stdCode);

	WTSHisTickData* get_ticks(uint32_t sid, const char* stdCode, uint32_t count);
	WTSTickData*	get_last_tick(uint32_t sid, const char* stdCode);

	WTSTickSlice*	get_tick_slice(uint32_t sid, const char* stdCode, uint32_t count);
	WTSKlineSlice*	get_kline_slice(uint32_t sid, const char* stdCode, const char* period, uint32_t count, uint32_t times = 1);

	void sub_tick(uint32_t sid, const char* code);

	double get_cur_price(const char* stdCode);

	double calc_fee(const char* stdCode, double price, int32_t qty, uint32_t offset);

	inline void setRiskMonitor(WtRiskMonPtr& monitor)
	{
		_risk_mon = monitor;
	}

	//////////////////////////////////////////////////////////////////////////
	//WtPortContext�ӿ�
	virtual WTSPortFundInfo* getFundInfo() override;

	virtual void setVolScale(double scale) override;

	virtual bool isInTrading() override;

	virtual void writeRiskLog(const char* fmt, ...) override;

	virtual uint32_t	getCurDate() override;
	virtual uint32_t	getCurTime() override;
	virtual uint32_t	getTradingDate() override;
	virtual uint32_t	transTimeToMin(uint32_t uTime) override{ return 0; }

public:
	virtual void init(WTSVariant* cfg, IBaseDataMgr* bdMgr, WtDataManager* dataMgr, IHotMgr* hotMgr);

	virtual void run(bool bAsync = false) = 0;

	virtual void on_tick(const char* stdCode, WTSTickData* curTick);

	virtual void on_bar(const char* stdCode, const char* period, uint32_t times, WTSBarStruct* newBar) = 0;

	virtual void handle_push_quote(WTSTickData* newTick, bool isHot);

	virtual void on_init(){}
	virtual void on_session_begin();
	virtual void on_session_end();

protected:
	/*
	 *	�����źŹ�����
	 */
	void		load_filters();
	
	/*
	 *	�Ƿ񱻹��˵���
	 *	����������Ǻ��ԵĻ�, �ͻ᷵��true, ������ض����λ, �ͻ᷵��false, ��Ŀ���λ��һ�����������Ĺ���ֵ
	 *
	 *	@sname		��������
	 *	@stdCode	��׼��Լ����
	 *	@targetPos	Ŀ���λ, �Ը�����Ϊ׼
	 *	
	 *	return		�Ƿ���˵���, ������˵���, �óֲ־Ͳ������������Ŀ���λ
	 */
	bool		is_filtered(const char* sname, const char* stdCode, int32_t& targetPos);

	void		load_fees(const char* filename);

	void		load_datas();

	void		save_datas();

	void		append_signal(const char* stdCode, int32_t qty);

	void		do_set_position(const char* stdCode, int32_t qty);

	void		task_loop();

	void		push_task(TaskItem task);

	void		update_fund_dynprofit();

	bool		init_riskmon(WTSVariant* cfg);


protected:
	uint32_t		_cur_date;	//��ǰ����
	uint32_t		_cur_time;		//��ǰʱ��, ��1������ʱ��, ����0900, ���ʱ���1��������0901, _cur_timeҲ����0901, �����Ϊ��CTA���淽��
	uint32_t		_cur_raw_time;	//��ǰ��ʵʱ��
	uint32_t		_cur_secs;	//��ǰ����, ��������
	uint32_t		_cur_tdate;	//��ǰ������

	IBaseDataMgr*	_base_data_mgr;	//�������ݹ�����
	IHotMgr*		_hot_mgr;		//����������
	WtDataManager*	_data_mgr;		//���ݹ�����

	typedef std::unordered_set<uint32_t> SIDSet;
	typedef std::unordered_map<std::string, SIDSet>	StraSubMap;
	StraSubMap		_tick_sub_map;	//tick���ݶ��ı�
	StraSubMap		_bar_sub_map;	//K�����ݶ��ı�

	std::unordered_set<std::string>		_subed_raw_codes;	//tick���ı�����ʵ����ģʽ��

	//////////////////////////////////////////////////////////////////////////
	//
	typedef struct _SigInfo
	{
		int32_t		_volumn;
		uint64_t	_gentime;

		_SigInfo()
		{
			_volumn = 0;
			_gentime = 0;
		}
	}SigInfo;
	typedef std::unordered_map<std::string, SigInfo>	SignalMap;
	SignalMap		_sig_map;

	//////////////////////////////////////////////////////////////////////////
	//�źŹ�����
	typedef enum tagFilterAction
	{
		FA_Ignore,		//����, ��ά��ԭ�в�λ
		FA_Redirect,	//�ض���ֲ�, ��ͬ����ָ��Ŀ���λ
		FA_None = 99
	} FilterAction;

	typedef struct _FilterItem
	{
		std::string		_key;		//�ؼ���
		FilterAction	_action;	//���˲���
		int32_t			_target;	//Ŀ���λ, ֻ�е�_actionΪFA_Redirect����Ч
	} FilterItem;

	typedef std::unordered_map<std::string, FilterItem>	FilterMap;
	FilterMap		_stra_filters;	//���Թ�����
	FilterMap		_code_filters;	//���������, ������Լ�����Ʒ�ִ���, ͬһʱ��ֻ��һ����Ч, ��Լ�������ȼ�����Ʒ�ִ���
	std::string		_filter_file;	//�����������ļ�
	uint64_t		_filter_timestamp;	//�������ļ�ʱ���



	//////////////////////////////////////////////////////////////////////////
	//������ģ��
	typedef struct _FeeItem
	{
		double	_open;
		double	_close;
		double	_close_today;
		bool	_by_volumn;

		_FeeItem()
		{
			memset(this, 0, sizeof(_FeeItem));
		}
	} FeeItem;
	typedef std::unordered_map<std::string, FeeItem>	FeeMap;
	FeeMap		_fee_map;
	

	WTSPortFundInfo*	_port_fund;

	//////////////////////////////////////////////////////////////////////////
	//�ֲ�����
	typedef struct _DetailInfo
	{
		bool		_long;
		double		_price;
		int32_t		_volumn;
		uint64_t	_opentime;
		uint32_t	_opentdate;
		double		_profit;

		_DetailInfo()
		{
			memset(this, 0, sizeof(_DetailInfo));
		}
	} DetailInfo;

	typedef struct _PosInfo
	{
		int32_t		_volumn;
		double		_closeprofit;
		double		_dynprofit;

		std::vector<DetailInfo> _details;

		_PosInfo()
		{
			_volumn = 0;
			_closeprofit = 0;
			_dynprofit = 0;
		}
	} PosInfo;
	typedef std::unordered_map<std::string, PosInfo> PositionMap;
	PositionMap		_pos_map;

	//////////////////////////////////////////////////////////////////////////
	//
	typedef std::unordered_map<std::string, double> PriceMap;
	PriceMap		_price_map;

	//��̨�����߳�, �ѷ�غ��ʽ�, �ֲָ��¶��ŵ�����߳���ȥ
	typedef std::queue<TaskItem>	TaskQueue;
	BoostThreadPtr	_thrd_task;
	TaskQueue		_task_queue;
	BoostUniqueMutex	_mtx_task;
	BoostCondition		_cond_task;
	bool			_terminated;

	typedef struct _RiskMonFactInfo
	{
		std::string		_module_path;
		DllHandle		_module_inst;
		IRiskMonitorFact*	_fact;
		FuncCreateRiskMonFact	_creator;
		FuncDeleteRiskMonFact	_remover;
	} RiskMonFactInfo;
	RiskMonFactInfo	_risk_fact;
	WtRiskMonPtr	_risk_mon;
	double			_risk_volscale;
	uint32_t		_risk_date;
};
NS_OTP_END