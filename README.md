# esteganografia_HiddenInPlainSight

Trabalho de Programação de Baixo Nível do curso de Ciência da Computação da PUCRS
Para compilar o projeto, basta executar o "make" no terminal

Este programa esconde uma imagem dentro de outra imagem (se a imagem a ser escondida tiver
até 1/4 do tamanho da outra). Este programa funciona apenas com imagens no formato PPM, formato esse
que pode ser aberto por softwares como o Gimp ou o IrfanView.

### Instruções de uso:
Obs.: não utilizar os símbolos "<" nem ">" na hora de especificar o nome. Utilizar underlines pare denotar espaços.
* Para esconder uma imagem em outra:
  ./hide <imagem_a_ser_escondida>.ppm <imagem_onde_esta_será_escondida>.ppm <nome_do_novo_arquivo_com_a_imagem_escondida>.ppm

* Para recuperar uma imagem escondida:
  ./hide <imagem>.ppm

