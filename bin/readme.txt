
	
raw2transedImage-NonVP.exe
	raw2transedImage-NonVP.exe xxx.jpg
	输出
		透视变换结果到transed_images/t_xxx.jpg
		四个点到PersPram.csv
	每次运行在PersPram.csv添加一行

getTransedImage.py
	调用raw2transedImage-NonVP.exe，输入来自images原始图片
	透视变换后图片保存在transed_images下，PersPram.csv每行一个图片的四个点数据

transedImage2context.exe
	不显示图片 transedImage2context.exe xxx.jpg
	显示图片   transedImage2context.exe xxx.jpg s
	必要的配置文件
		pca_avg
		pca_sigma
		pca_W
		Param_GSM
		regression_param
	输出
		FeatureVector
		pca_pc
		context
	每次运行在context添加一行
getContexts.py
	调用transedImage2context.exe，输入来自transed_images下的透视变换后的图片
	归化到0-1的平均视线保存在context.txt，每个图片一行

contextpluspers.py
	合并context.txt与PersPram.csv到contextWithPers.txt，格式：图片路径,context,L0,L1,R0,R1

context2newSM.exe
	context2newSM.exe xxx.jpg context L0 L1 R0 R1
	再加个参数s表示显示结果图片，作为单次测试使用

getSM.py
	调用context2newSM.exe，输入图片来自images原始图片，context与PersPram从contextWithPers.txt读取
	对每张图，输出sm,context,dst三个图到result_images目录

========================================
整体流程，脚本运行顺序
	从images下的原始图片开始
	运行getTransedImage.py手动对所有图片透视变换
	运行getContexts.py获得模型预测的平均视线位置信息
	运行contextpluspers.py合并context.txt与PersPram.csv
	运行getSM.py生成最终结果图