/**
  \defgroup kernel
  @{
  \file kernel.h
  \brief Header file for Kernel APIs

  This header file contains the API function prototypes
  for the kernel.
*/

/**
  \brief Kernel event container

  Contains all of the possible kernel events that may be associated
  with threads
*/
typedef enum{
  kernel_UART_RX,
  kernel_UART_TX,
  kernel_SSI_RX,
  kernel_SSI_TX,
  kernel_SYSTICK,
  kernel_GPIOA,
  kernel_GPIOB,
  kernel_GPIOC,
  kernel_GPIOD,
  kernel_GPIOE,
  kernel_GPIOF,
  kernel_GPIOG,
}t_kernelEventType

/**
  \brief Yield timeslot back to kernel

  This function is called when the running thread no longer has
  work to do, and may return to the kernel so other threads may execute.
*/
void yield(void);

/**
  \brief Add a thread to the kernel, with a specified event to service.

  A thread is added to the kernel and will run when a given event happens

  \param fx_ptr a pointer to the function that is the thread to be added
  \param event the event type to trigger the thread to run

*/
void addThread(void* fx_ptr, t_kernelEventType event);


/** @}*/
