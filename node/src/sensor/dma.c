
#define BUFFER_SIZE 128
extern DMA_HandleTypeDef hdma_uart2_rx;
extern UART_HandleTypeDef UartHandle;

extern uint8_t rx_len;
extern uint8_t recv_end_flag;
extern uint8_t rx_buffer[128];
/** 
  * Enable DMA controller clock
  */
void DMAInit(void) 
{
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __USART2_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/
  
    hdma_uart2_rx.Instance = DMA2_Stream2;
    hdma_uart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart2_rx.Init.Mode = DMA_NORMAL;
    hdma_uart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_uart2_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_uart2_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_uart2_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_uart2_rx);

    __HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);

  /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(huart->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* Peripheral clock enable */
    __USART3_CLK_ENABLE();
  
    /**USART3 GPIO Configuration    
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* Peripheral DMA DeInit*/
    HAL_DMA_DeInit(huart->hdmarx);

    /* Peripheral interrupt DeInit*/
    HAL_NVIC_DisableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(huart->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration    
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* Peripheral interrupt DeInit*/
    HAL_NVIC_DisableIRQ(USART3_IRQn);

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }

}

/**
* @brief This function handles DMA2 Stream2 global interrupt.
*/
void DMA1_Channel1_IRQn(void)
{
  /* USER CODE BEGIN DMA2_Stream2_IRQn 0 */

  /* USER CODE END DMA2_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_uart2_rx);
  /* USER CODE BEGIN DMA2_Stream2_IRQn 1 */

  /* USER CODE END DMA2_Stream2_IRQn 1 */
}